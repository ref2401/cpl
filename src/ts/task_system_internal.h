#ifndef TS_TASK_SYSTEM_INTERNAL_H_
#define TS_TASK_SYSTEM_INTERNAL_H_

#include <atomic>
#include <vector>
#include "ts/concurrent_queue.h"
#include "ts/task_system.h"


namespace ts {

class fiber_wait_list final {
public:

	explicit fiber_wait_list(size_t fiber_count);

	fiber_wait_list(fiber_wait_list&&) = delete;
	fiber_wait_list& operator=(fiber_wait_list&&) = delete;


	// Puts the given pair of a fiber and its wait counter to the underlying list.
	// Does not check whether the specified fiber is already in the list.
	void push(void* p_fiber, std::atomic_size_t* p_wait_counter);

	// Iterates over the wait list searching for a fiber whose wait counter equals to zero.
	// Returns true if such a fiber has been found, p_out_fiber will store the value.
	bool try_pop(void*& p_out_fiber);

private:

	struct list_entry final {
		void* p_fiber = nullptr;
		std::atomic_size_t* p_wait_counter = nullptr;
	};


	std::vector<list_entry> wait_list_;
	std::mutex mutex_;
	size_t push_index_ = 0;
};

struct task final {
	std::function<void()> func;
};

struct task_system_state final {
	task_system_state(size_t queue_size, size_t queue_size_immediate);

	concurrent_queue<task> 	queue;
	concurrent_queue<task> 	queue_immediate;
	std::atomic_bool		exec_flag;
};

class task_system final {
public:

	explicit task_system(task_system_desc desc);

private:

	std::vector<std::thread>	worker_threads_;
	fiber_wait_list				wait_list_;
	//fiber::fiber_pool 			fiber_pool_(fiber_count, fiber_func);
	//task_system_state			state_;
	//task_system_report 			report_;
};

} // namespace ts

#endif // TS_TASK_SYSTEM_INTERNAL_H_
