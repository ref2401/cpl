#ifndef TS_TASK_SYSTEM_INTERNAL_H_
#define TS_TASK_SYSTEM_INTERNAL_H_

#include <vector>
#include "ts/concurrent_queue.h"
#include "ts/fiber.h"
#include "ts/task_system.h"


namespace ts {

class fiber_wait_list final {
public:

	explicit fiber_wait_list(size_t fiber_count);

	fiber_wait_list(fiber_wait_list&&) = delete;
	fiber_wait_list& operator=(fiber_wait_list&&) = delete;


	// Puts the given pair of a fiber and its wait counter to the underlying list.
	// Does not check whether the specified fiber is already in the list.
	void push(void* p_fiber, const std::atomic_size_t* p_wait_counter);

	// Iterates over the wait list searching for a fiber whose wait counter equals to zero.
	// Returns true if such a fiber has been found, p_out_fiber will store the value.
	bool try_pop(void*& p_out_fiber);

private:

	struct list_entry final {
		void*						p_fiber = nullptr;
		const std::atomic_size_t*	p_wait_counter = nullptr;
	};


	std::vector<list_entry>	wait_list_;
	std::mutex				mutex_;
	size_t					push_index_ = 0;
};

struct task final {
	std::function<void()>	func;
	std::atomic_size_t*		p_wait_counter = nullptr;
};

struct task_system final {
	task_system(size_t queue_size, size_t queue_immediate_size, size_t worker_thread_count);

	concurrent_queue<task>	queue;
	concurrent_queue<task>	queue_immediate;
	std::atomic_bool		exec_flag;
	task_system_report 		report;
	const size_t			worker_thread_count;
};

// Represents thread local communication channel between the thread controller fiber
// and a worker fiber which is executed in the current thread.
struct worker_fiber_context final {
	static thread_local void* 						p_controller_fiber;
	static thread_local const std::atomic_size_t*	p_wait_list_counter;
	static thread_local std::exception_ptr			exception;
};


inline void exec_task(task& t)
{
	t.func();

	if (t.p_wait_counter) {
		assert(*t.p_wait_counter > 0);
		--(*t.p_wait_counter);
	}
}

} // namespace ts

#endif // TS_TASK_SYSTEM_INTERNAL_H_
