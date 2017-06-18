#include "ts/task_system.h"
#include "ts/task_system_internal.h"

#include <memory>


namespace {

using namespace ts;
std::unique_ptr<task_system> gp_task_system;


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
}

void worker_thread_func(fiber_pool& fiber_pool, fiber_wait_list& fiber_wait_list, std::atomic_bool& exec_flag)
{
	thread_main_fiber tmf;

}

} // namespace


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
	: state_(desc.queue_size, desc.queue_immediate_size),
	fiber_pool_(desc.fiber_count, worker_fiber_func, desc.fiber_staeck_byte_count, &state_),
	fiber_wait_list_(desc.fiber_count)
{
	worker_threads_.reserve(desc.thread_count);
	for (size_t i = 0; i < desc.thread_count; ++i) {
		worker_threads_.emplace_back(worker_thread_func,
			std::ref(fiber_pool_),
			std::ref(fiber_wait_list_),
			std::ref(state_.exec_flag));
	}
}

task_system::~task_system() noexcept
{
	state_.exec_flag = false;
	state_.queue.set_wait_allowed(false);
	state_.queue_immediate.set_wait_allowed(false);

	for (auto& th : worker_threads_)
		th.join();
}

void task_system::run(task* p_tasks, size_t count)
{
	assert(p_tasks);
	assert(count > 0);


}

// ----- funcs -----

void init_task_system(const task_system_desc& desc)
{
	assert(!gp_task_system);
	assert(is_valid_task_system_desc(desc));

	gp_task_system = std::make_unique<task_system>(desc);
}

task_system_report terminate_task_system() 
{
	assert(gp_task_system);

	auto report = gp_task_system->report();
	gp_task_system = nullptr;

	return report;
}

void run(task* p_tasks, size_t count)
{
	assert(gp_task_system);
	gp_task_system->run(p_tasks, count);
}

size_t thread_count() noexcept
{
	assert(gp_task_system);
	return gp_task_system->thread_count();
}

} // namespace ts
