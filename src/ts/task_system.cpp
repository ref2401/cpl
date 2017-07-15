#include "ts/task_system.h"

#include <memory>
#include "ts/fiber.h"
#include "ts/concurrent_queue.h"


namespace {

using namespace ts;

struct task final {
	std::function<void()>	func;
	std::atomic_size_t*		p_wait_counter = nullptr;
};

// Task system state.
struct tss final {
	// Task system global 'fields'.
	// 
	static concurrent_queue<task>*	p_queue;
	static concurrent_queue<task>*	p_queue_immediate;
	static exception_slot			exception_slot;
	static task_system_report		report;
	static std::atomic_bool			exec_flag;

	// The following fields represents thread local communication channel between 
	// the thread controller fiber and a worker fiber which is executed in the current thread.
	// 
	static thread_local void* 						p_controller_fiber;
	static thread_local const std::atomic_size_t*	p_wait_list_counter;
};

concurrent_queue<task>*					tss::p_queue = nullptr;
concurrent_queue<task>*					tss::p_queue_immediate = nullptr;
exception_slot							tss::exception_slot;
task_system_report						tss::report;
std::atomic_bool						tss::exec_flag = false;
thread_local void*						tss::p_controller_fiber = nullptr;
thread_local const std::atomic_size_t*	tss::p_wait_list_counter = nullptr;

// ----- funcs ------

inline void exec_task(task& t)
{
	t.func();

	if (t.p_wait_counter) {
		assert(*t.p_wait_counter > 0);
		--(*t.p_wait_counter);
	}
}

void kernel_fiber_func(void* data)
{
	kernel_func_t p_kernel_func = static_cast<kernel_func_t>(data);

	try {
		p_kernel_func();
	}
	catch (...) {
		tss::exception_slot.set_exception(std::current_exception());
	}

	switch_to_fiber(tss::p_controller_fiber);
}

void kernel_thread_func(kernel_func_t p_kernel_func, fiber_pool& fiber_pool,
	fiber_wait_list& fiber_wait_list)
{
	thread_fiber_nature			tfn;
	fiber						kernel_fiber(kernel_fiber_func, 1024, p_kernel_func);
	const std::atomic_size_t*	p_kernel_wait_counter = nullptr;
	void* 						p_fiber_to_exec = kernel_fiber.p_handle;


	// init fiber execution context
	tss::p_controller_fiber = tfn.p_handle;
	tss::p_wait_list_counter = nullptr;

	// main loop
	while (tss::exec_flag) {
		switch_to_fiber(p_fiber_to_exec);
		if (tss::exception_slot.has_exception()) {
			tss::exec_flag = false;
			return;
		}

		if (tss::p_wait_list_counter) {
			// Fiber's code has called ts::wait_for.
			// The current fiber must be put into the wait list.
			if (p_kernel_wait_counter) {
				fiber_wait_list.push(p_fiber_to_exec, tss::p_wait_list_counter);
			}
			else {
				p_kernel_wait_counter = tss::p_wait_list_counter;
				assert(p_fiber_to_exec == kernel_fiber.p_handle);
			}

			p_fiber_to_exec = fiber_pool.pop();
			tss::p_wait_list_counter = nullptr;
		}
		else {
			// Fiber's code has finished its current tasks. No wait request occured.
			// Check if the kernel fiber is completed. If so, then stop the task system.
			if (p_kernel_wait_counter == nullptr) {
				tss::exec_flag = false;
				return;
			}
			
			// Check if any of the waiting fibers are ready.
			void* p_fbr = nullptr;

			// If the kernel fiber is NOT ready we are going to exec any fiber from the wait list.
			if (*p_kernel_wait_counter > 0) {
				const bool r = fiber_wait_list.try_pop(p_fbr);
			}
			else {
				// The kernel fiber is ready.
				p_fbr = kernel_fiber.p_handle;
				p_kernel_wait_counter = nullptr;
			}

			// If a waiting fiber has been found we return the current fiber back to the pool.
			if (p_fbr) {
				fiber_pool.push_back(p_fiber_to_exec);
				p_fiber_to_exec = p_fbr;
			}
		}
	} // while
}

void worker_fiber_func(void*)
{
	while (tss::exec_flag) {
		// drain queue_immediate

		// process regular tasks
		task t;
		const bool r = tss::p_queue->try_pop(t);
		if (r) {
			try {
				exec_task(t);
			}
			catch (...) {
				tss::exception_slot.set_exception(std::current_exception());
			}
		}

		switch_to_fiber(tss::p_controller_fiber);
	}

	switch_to_fiber(tss::p_controller_fiber);
}

void worker_thread_func(fiber_pool& fiber_pool, fiber_wait_list& fiber_wait_list)
{
	thread_fiber_nature	tmf;
	void* 				p_fiber_to_exec = fiber_pool.pop();

	// init fiber execution context (thread_local part of the tss)
	tss::p_controller_fiber = tmf.p_handle;
	tss::p_wait_list_counter = nullptr;

	// main loop
	while (tss::exec_flag) {
		switch_to_fiber(p_fiber_to_exec);
		if (tss::exception_slot.has_exception()) {
			tss::exec_flag = false;
			return;
		}

		if (tss::p_wait_list_counter) {
			// Fiber's code has called ts::wait_for.
			// The current fiber must be put into the wait list.
			fiber_wait_list.push(p_fiber_to_exec, tss::p_wait_list_counter);
			tss::p_wait_list_counter = nullptr;

			p_fiber_to_exec = fiber_pool.pop();
			assert(p_fiber_to_exec);
		}
		else {
			// Fiber's code has finished its current tasks. No wait request occured.
			// Check if any of the waiting fibers are ready.
			void* p_fpr;
			const bool r = fiber_wait_list.try_pop(p_fpr);
			if (r) {
				fiber_pool.push_back(p_fiber_to_exec);
				p_fiber_to_exec = p_fpr;
			}
		}
	} // while
}

} // namespace


namespace ts {

// ----- funcs -----

task_system_report launch_task_system(const task_system_desc& desc, kernel_func_t p_kernel_func)
{
	assert(!tss::p_queue);
	assert(is_valid_task_system_desc(desc));
	assert(p_kernel_func);

	try {
		// init the task system
		concurrent_queue<task>	queue(desc.queue_size);
		concurrent_queue<task>	queue_immediate(desc.queue_immediate_size);
		fiber_pool				fiber_pool(desc.fiber_count, worker_fiber_func, desc.fiber_stack_byte_count);
		fiber_wait_list			fiber_wait_list(desc.fiber_count);
		
		tss::p_queue = &queue;
		tss::p_queue_immediate = &queue_immediate;
		tss::exec_flag = true;
		
		// spawn new worker threads if needed
		// desc.thread_count - 1 because 1 stands for the kernel thread
		std::vector<std::thread> worker_threads;
		worker_threads.reserve(desc.thread_count - 1);
		for (size_t i = 0; i < desc.thread_count - 1; ++i) {
			worker_threads.emplace_back(worker_thread_func,
				std::ref(fiber_pool),
				std::ref(fiber_wait_list));
		}

		// run the kernel thread's func. the kernel func is executed here.
		kernel_thread_func(p_kernel_func, fiber_pool, fiber_wait_list);
		assert(!tss::exec_flag);

		// finilize the task system
		queue.set_wait_allowed(false);
		queue_immediate.set_wait_allowed(false);
		for (auto& th : worker_threads)
			th.join();

		// only after all the threads have been joined we may rethrow.
		if (tss::exception_slot.has_exception())
			std::rethrow_exception(tss::exception_slot.exception());
		
		return tss::report;
	}
	catch (...) {
		std::throw_with_nested(std::runtime_error("Task system execution error."));
	}
}

void run(std::function<void()>* p_funcs, size_t count, std::atomic_size_t* p_wait_counter)
{
	assert(tss::p_queue);
	assert(p_funcs);
	assert(count > 0);

	if (p_wait_counter)
		*p_wait_counter = count;

	for (size_t i = 0; i < count; ++i)
		tss::p_queue->emplace(std::move(p_funcs[i]), p_wait_counter);

	tss::report.task_count += count;
}

void wait_for(const std::atomic_size_t& wait_counter)
{
	assert(current_fiber() != tss::p_controller_fiber);

	if (wait_counter == 0) return;

	tss::p_wait_list_counter = &wait_counter;
	switch_to_fiber(tss::p_controller_fiber);
}

} // namespace ts
