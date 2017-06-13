#ifndef TS_TASK_SYSTEM_H_
#define TS_TASK_SYSTEM_H_

#include <atomic>
#include "ts/concurrent_queue.h"


namespace ts {

class fiber_wait_list final {
public:

	fiber_wait_list(size_t fiber_count);

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

} // namespace ts

#endif // TS_TASK_SYSTEM_H_
