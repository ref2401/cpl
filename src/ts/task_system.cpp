#include "ts/task_system.h"
#include "ts/task_system_internal.h"



namespace ts {

// ----- fiber_wait_list -----

fiber_wait_list::fiber_wait_list(size_t fiber_count)
{
	wait_list_.resize(fiber_count);
}

void fiber_wait_list::push(void* p_fiber, std::atomic_size_t* p_wait_counter)
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
	: wait_list_(desc.fiber_count)
{
	assert(desc.thread_count > 0);
	assert(desc.fiber_count > 0);
	assert(desc.queue_size > 0);
	assert(desc.queue_immediate_size > 0);

	//worker_threads.reserve(desc.thread_count);
}

// ----- funcs -----

void init_task_system(const task_system_desc& desc)
{
}

task_system_report terminate_task_system() 
{
	return {};
}

void run(task_desc* p_tasks, size_t count)
{
}

size_t thread_count() noexcept
{
	return 0;
}

} // namespace ts
