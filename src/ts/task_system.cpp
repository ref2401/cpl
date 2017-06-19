#include "ts/task_system.h"
#include "ts/task_system_internal.h"

#include <memory>


namespace {

using namespace ts;
std::unique_ptr<task_system> gp_task_system;

// Represents thread local communication channel between the thread main fiber
// and a worker fiber which is executed in the current thread.
struct worker_fiber_context final {
	static thread_local void* 						p_thread_main_fiber;
	static thread_local const std::atomic_size_t*	p_wait_list_counter;
};

thread_local void*						worker_fiber_context::p_thread_main_fiber;
thread_local const std::atomic_size_t*	worker_fiber_context::p_wait_list_counter;


inline void exec_task(task& t)
{
	t.func();

	if (t.p_wait_counter) {
		assert(*t.p_wait_counter > 0);
		--(*t.p_wait_counter);
	}
}

bool is_valid_task_system_desc(const task_system_desc& desc) noexcept
{
	return (desc.thread_count > 0)
		&& (desc.fiber_count > 0)
		&& (desc.queue_size > 0)
		&& (desc.queue_immediate_size > 0);
}

void worker_fiber_func(void* data)
{
	task_system_state& state = *static_cast<task_system_state*>(data);

	while (state.exec_flag) {
		// drain queue_immediate

		// process regular tasks
		task t;
		const bool r = state.queue.try_pop(t);
		if (r) 
			exec_task(t);

		switch_to_fiber(worker_fiber_context::p_thread_main_fiber);
	}

	switch_to_fiber(worker_fiber_context::p_thread_main_fiber);
}

void worker_thread_func(fiber_pool& fiber_pool, fiber_wait_list& fiber_wait_list, std::atomic_bool& exec_flag)
{
	thread_main_fiber	tmf;
	void* 				p_fiber_to_exec = fiber_pool.pop();

	// init fiber execution context
	worker_fiber_context::p_thread_main_fiber = tmf.p_fiber;
	worker_fiber_context::p_wait_list_counter = nullptr;

	// main loop
	while (exec_flag) {
		switch_to_fiber(p_fiber_to_exec);

		if (worker_fiber_context::p_wait_list_counter) {
			fiber_wait_list.push(p_fiber_to_exec, worker_fiber_context::p_wait_list_counter);
			worker_fiber_context::p_wait_list_counter = nullptr;

			p_fiber_to_exec = fiber_pool.pop();
		}
		else {
			// waiting fiber
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

// ----- task_system_state -----

task_system_state::task_system_state(size_t queue_size, size_t queue_immediate_size)
	: queue(queue_size), queue_immediate(queue_immediate_size), exec_flag(true)
{}

// ----- task_system -----

task_system::task_system(task_system_desc desc)
	: state(desc.queue_size, desc.queue_immediate_size),
	fiber_pool(desc.fiber_count, worker_fiber_func, desc.fiber_stack_byte_count, &state),
	fiber_wait_list(desc.fiber_count)
{
	// spawn new worker threads if needed
	const size_t tc = desc.thread_count - 1;
	worker_threads.reserve(tc);

	for (size_t i = 0; i < tc; ++i) {
		worker_threads.emplace_back(worker_thread_func,
			std::ref(fiber_pool),
			std::ref(fiber_wait_list),
			std::ref(state.exec_flag));
	}
}

// ----- funcs -----

task_system_report launch_task_system(const task_system_desc& desc, topmost_func_t p_topmost_func)
{
	assert(!gp_task_system);
	assert(is_valid_task_system_desc(desc));
	assert(p_topmost_func);

	gp_task_system = std::make_unique<task_system>(desc);
	
	try {

		{
			task_desc t(p_topmost_func, std::ref(gp_task_system->state.exec_flag));
			run(&t, 1, nullptr);
		}

		worker_thread_func(gp_task_system->fiber_pool, 
			gp_task_system->fiber_wait_list, gp_task_system->state.exec_flag);
		
		gp_task_system->state.exec_flag = false;
		gp_task_system->state.queue.set_wait_allowed(false);
		gp_task_system->state.queue_immediate.set_wait_allowed(false);

		for (auto& th : gp_task_system->worker_threads)
			th.join();
		
		auto report = gp_task_system->report;
		gp_task_system = nullptr;
		return report;
	}
	catch (...) {
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
		gp_task_system->state.queue.emplace(std::move(p_tasks[i].func), p_wait_counter);

	gp_task_system->report.task_count += count;
}

size_t thread_count() noexcept
{
	assert(gp_task_system);
	return gp_task_system->worker_threads.size() + 1;
}

void wait_for(const std::atomic_size_t& wait_counter)
{
	assert(gp_task_system);
	assert(current_fiber() != worker_fiber_context::p_thread_main_fiber);

	if (wait_counter == 0) return;

	worker_fiber_context::p_wait_list_counter = &wait_counter;
	switch_to_fiber(worker_fiber_context::p_thread_main_fiber);
}

} // namespace ts
