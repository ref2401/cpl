#ifndef CPL_CONCURRENCY_QUEUE_H_
#define CPL_CONCURRENCY_QUEUE_H_

#include <cassert>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <type_traits>
#include "cpl/utility.h"


namespace cpl {

template<typename T>
class concurrent_queue final {
public:

	explicit concurrent_queue(size_t size_limit);

	concurrent_queue(const concurrent_queue&) = delete;
	concurrent_queue& operator=(const concurrent_queue&) = delete;


	bool empty() const;

	size_t size() const;

	template<typename U>
	void push(U&& v);

	// Tries to pop a value from the queue. If the queue is empty returns false and leaves out_v unchanged.
	bool try_pop(T& out_v);

	// Blocks if the queue empty and it's allowed to wait (wait_allowed == true)
	bool wait_pop(T& out_v, const std::atomic_bool& wait_allowed = true);


private:

	ring_buffer<T> queue;
	mutable std::mutex mutex;
	std::condition_variable not_empty_condition;
};


template<typename T>
concurrent_queue<T>::concurrent_queue(size_t size_limit)
	: queue(size_limit)
{}

template<typename T>
bool concurrent_queue<T>::empty() const
{
	std::lock_guard<std::mutex> lock(mutex);
	return queue.empty();
}

template<typename T>
size_t concurrent_queue<T>::size() const
{
	std::lock_guard<std::mutex> lock(mutex);
	return queue.size();
}

template<typename T>
template<typename U>
void concurrent_queue<T>::push(U&& v)
{
	static_assert(std::is_same<T, std::remove_reference<U>::type>::value, "U must be implicitly convertible to T.");

	{
		std::lock_guard<std::mutex> lock(mutex);
		bool res = queue.try_push(std::forward<U>(v));
		assert(res);
	}

	not_empty_condition.notify_one();
}

template<typename T>
bool concurrent_queue<T>::try_pop(T& out_v)
{
	std::lock_guard<std::mutex> lock(mutex);
	if (queue.empty()) return false;

	return queue.try_pop(out_v);
}

template<typename T>
bool concurrent_queue<T>::wait_pop(T& out_v, const std::atomic_bool& wait_allowed)
{
	// wait until the queue is empty and waiting is allowed.
	std::unique_lock<std::mutex> lock(mutex);
	not_empty_condition.wait(lock, [this, &wait_allowed] { return !(queue.empty() && wait_allowed); });

	if (!wait_allowed) return false;

	return queue.try_pop(out_v);
}

} // namespace cpl

#endif // CPL_CONCURRENCY_QUEUE_H_
