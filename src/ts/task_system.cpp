#include "ts/task_system.h"
#include "ts/task_system_internal.h"

#include <memory>


namespace {

using namespace ts;
ts::task_system* gp_task_system;

void kernel_fiber_func(void* data)
{
	kernel_func_t p_kernel_func = static_cast<kernel_func_t>(data);

	try {
		p_kernel_func();
	}
	catch (...) {
		worker_fiber_context::exception = std::current_exception();
	}

	switch_to_fiber(worker_fiber_context::p_controller_fiber);
}

void kernel_thread_func(kernel_func_t p_kernel_func, fiber_pool& fiber_pool,
	fiber_wait_list& fiber_wait_list, exception_slot& exception_slot, std::atomic_bool& exec_flag)
{
	thread_fiber_nature			tfn;
	fiber						kernel_fiber(kernel_fiber_func, 1024, p_kernel_func);
	const std::atomic_size_t*	p_kernel_wait_counter = nullptr;
	void* 						p_fiber_to_exec = kernel_fiber.p_handle;


	// init fiber execution context
	worker_fiber_context::p_controller_fiber = tfn.p_handle;
	worker_fiber_context::p_wait_list_counter = nullptr;

	// main loop
	while (exec_flag) {
		switch_to_fiber(p_fiber_to_exec);
		if (worker_fiber_context::exception) {
			exception_slot.set_exception(worker_fiber_context::exception);
			exec_flag = false;
			return;
		}

		if (worker_fiber_context::p_wait_list_counter) {
			// Fiber's code has called ts::wait_for.
			// The current fiber must be put into the wait list.
			if (p_kernel_wait_counter) {
				fiber_wait_list.push(p_fiber_to_exec, worker_fiber_context::p_wait_list_counter);
			}
			else {
				p_kernel_wait_counter = worker_fiber_context::p_wait_list_counter;
				assert(p_fiber_to_exec == kernel_fiber.p_handle);
			}

			p_fiber_to_exec = fiber_pool.pop();
			worker_fiber_context::p_wait_list_counter = nullptr;
		}
		else {
			// Fiber's code has finished its current tasks. No wait request occured.
			// Check if the kernel fiber is completed. If so, then stop the task system.
			if (p_kernel_wait_counter == nullptr) {
				exec_flag = false;
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

void worker_fiber_func(void* data)
{
	task_system& state = *static_cast<task_system*>(data);

	while (state.exec_flag) {
		// drain queue_immediate

		// process regular tasks
		task t;
		const bool r = state.queue.try_pop(t);
		if (r)
			exec_task(t);

		switch_to_fiber(worker_fiber_context::p_controller_fiber);
	}

	switch_to_fiber(worker_fiber_context::p_controller_fiber);
}

void worker_thread_func(fiber_pool& fiber_pool, fiber_wait_list& fiber_wait_list,
	exception_slot& exception_slot, std::atomic_bool& exec_flag)
{
	thread_fiber_nature	tmf;
	void* 				p_fiber_to_exec = fiber_pool.pop();

	// init fiber execution context
	worker_fiber_context::p_controller_fiber = tmf.p_handle;
	worker_fiber_context::p_wait_list_counter = nullptr;
	worker_fiber_context::exception = nullptr;

	// main loop
	while (exec_flag) {
		switch_to_fiber(p_fiber_to_exec);
		//if (worker_fiber_context::exception) {

		//}

		if (worker_fiber_context::p_wait_list_counter) {
			// Fiber's code has called ts::wait_for.
			// The current fiber must be put into the wait list.
			fiber_wait_list.push(p_fiber_to_exec, worker_fiber_context::p_wait_list_counter);
			worker_fiber_context::p_wait_list_counter = nullptr;

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

// ----- fiber_wait_list -----

fiber_wait_list::fiber_wait_list(size_t fiber_count)
{
	wait_list_.resize(fiber_count);
}

void fiber_wait_list::push(void* p_fiber, const std::atomic_size_t* p_wait_counter)
{
	assert(p_fiber);
	assert(p_wait_counter);
	assert(*p_wait_counter > 0);

	std::lock_guard<std::mutex> lock(mutex_);
	assert(push_index_ < wait_list_.size());

	wait_list_[push_index_] = list_entry { p_fiber, p_wait_counter };
	++push_index_;
}

bool fiber_wait_list::try_pop(void*& p_out_fiber)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (push_index_ == 0) return false;

	for (size_t i = push_index_; i > 0; --i) {
		list_entry& e = wait_list_[i - 1];
		if (*e.p_wait_counter > 0) continue;

		// remove i - 1 fiber from the list
		p_out_fiber = e.p_fiber;
		e.p_fiber = nullptr;
		e.p_wait_counter = nullptr;

		if (i != push_index_)
			std::swap(e, wait_list_[push_index_ - 1]);

		--push_index_;
		return true;
	}

	return false;
}

// ----- task_system -----

task_system::task_system(size_t queue_size, size_t queue_immediate_size,
	size_t worker_thread_count)
	: queue(queue_size), 
	queue_immediate(queue_immediate_size), 
	exec_flag(true),
	worker_thread_count(worker_thread_count)
{}

// ----- worker_fiber_context -----

thread_local void*						worker_fiber_context::p_controller_fiber;
thread_local const std::atomic_size_t*	worker_fiber_context::p_wait_list_counter;
thread_local std::exception_ptr			worker_fiber_context::exception;

// ----- funcs -----

task_system_report launch_task_system(const task_system_desc& desc, kernel_func_t p_kernel_func)
{
	assert(!gp_task_system);
	assert(is_valid_task_system_desc(desc));
	assert(p_kernel_func);

	try {

		// init the task system
		task_system		state(desc.queue_size, desc.queue_immediate_size, desc.thread_count);
		fiber_pool		fiber_pool(desc.fiber_count, worker_fiber_func, desc.fiber_stack_byte_count, &state);
		fiber_wait_list	fiber_wait_list(desc.fiber_count);
		exception_slot	exception_slot;
		gp_task_system = &state;
		
		// spawn new worker threads if needed
		// desc.thread_count - 1 because 1 stands for the kernel thread
		std::vector<std::thread> worker_threads;
		worker_threads.reserve(desc.thread_count - 1);
		for (size_t i = 0; i < desc.thread_count - 1; ++i) {
			worker_threads.emplace_back(worker_thread_func,
				std::ref(fiber_pool),
				std::ref(fiber_wait_list),
				std::ref(exception_slot),
				std::ref(state.exec_flag));
		}

		// run the kernel thread's func. the kernel func is executed here.
		kernel_thread_func(p_kernel_func, fiber_pool, fiber_wait_list, exception_slot, gp_task_system->exec_flag);
		assert(!state.exec_flag);

		// finilize the task system
		state.queue.set_wait_allowed(false);
		state.queue_immediate.set_wait_allowed(false);
		for (auto& th : worker_threads)
			th.join();

		// only after all the threads have been joined we may rethrow.
		if (exception_slot.has_exception())
			std::rethrow_exception(exception_slot.exception());
		
		auto report = gp_task_system->report;
		gp_task_system = nullptr;
		return report;
	}
	catch (...) {
		gp_task_system = nullptr;
		std::throw_with_nested(std::runtime_error("Task system execution error."));
	}
}

void run(task_desc* p_tasks, size_t count, std::atomic_size_t* p_wait_counter)
{
	assert(gp_task_system);
	assert(p_tasks);
	assert(count > 0);

	if (p_wait_counter)
		*p_wait_counter = count;

	for (size_t i = 0; i < count; ++i)
		gp_task_system->queue.emplace(std::move(p_tasks[i].func), p_wait_counter);

	gp_task_system->report.task_count += count;
}

size_t thread_count() noexcept
{
	assert(gp_task_system);
	return gp_task_system->worker_thread_count;
}

void wait_for(const std::atomic_size_t& wait_counter)
{
	assert(gp_task_system);
	assert(current_fiber() != worker_fiber_context::p_controller_fiber);

	if (wait_counter == 0) return;

	worker_fiber_context::p_wait_list_counter = &wait_counter;
	switch_to_fiber(worker_fiber_context::p_controller_fiber);
}

} // namespace ts
