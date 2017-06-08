#include "ts/task_system.h"



namespace ts {

// ----- fiber_wait_list -----

fiber_wait_list::fiber_wait_list(size_t fiber_count)
{
	wait_list_.resize(fiber_count);
}

void fiber_wait_list::push(void* p_fiber, std::atomic_size_t* wait_counter)
{
	assert(p_fiber);
	assert(wait_counter);
	assert(*wait_counter > 0);

	std::lock_guard<std::mutex> lock(mutex_);
	wait_list_.emplace_back(p_fiber, p_exec_obj);
}

} // namespace ts
