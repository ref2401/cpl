#ifndef TS_CONCURRENCY_QUEUE_H_
#define TS_CONCURRENCY_QUEUE_H_

#include <cassert>
#include <atomic>
#include <condition_variable>
#include <iterator>
#include <mutex>
#include <queue>
#include "ts/utility.h"


namespace ts {

template<typename T>
class concurrent_queue final {
public:

	explicit concurrent_queue(size_t size_limit);

	concurrent_queue(const concurrent_queue&) = delete;
	concurrent_queue& operator=(const concurrent_queue&) = delete;


	bool empty() const;

	size_t size() const;

	bool wait_allowed() const noexcept
	{
		return wait_allowed_;
	}

	void set_wait_allowed(bool flag);

	template<typename... Args>
	void emplace(Args&&... args);

	template<typename U>
	void push(U&& v);

	template<typename InputIt>
	void push(InputIt b, InputIt e);

	// Tries to pop a value from the queue. If the queue is empty returns false and leaves out_v unchanged.
	bool try_pop(T& out_v);

	// Blocks if the queue empty and it's allowed to wait (wait_allowed == true).
	bool wait_pop(T& out_v);


private:

	ring_buffer<T>			queue_;
	mutable std::mutex		mutex_;
	std::condition_variable not_empty_condition_;
	std::atomic_bool		wait_allowed_;
};


template<typename T>
concurrent_queue<T>::concurrent_queue(size_t size_limit)
	: queue_(size_limit),
	wait_allowed_(true)
{}

template<typename T>
bool concurrent_queue<T>::empty() const
{
	std::lock_guard<std::mutex> lock(mutex_);
	return queue_.empty();
}

template<typename T>
size_t concurrent_queue<T>::size() const
{
	std::lock_guard<std::mutex> lock(mutex_);
	return queue_.size();
}

template<typename T>
void concurrent_queue<T>::set_wait_allowed(bool flag)
{
	bool prev = wait_allowed_.exchange(flag);

	// notify all only if flag is false and the previous value was true.
	if (!flag && prev != flag)
		not_empty_condition_.notify_all();
}

template<typename T>
template<typename... Args>
void concurrent_queue<T>::emplace(Args&&... args)
{
	{
		std::lock_guard<std::mutex> lock(mutex_);
		bool res = queue_.try_emplace(std::forward<Args>(args)...);
		assert(res);
	}

	not_empty_condition_.notify_one();
}

template<typename T>
template<typename U>
void concurrent_queue<T>::push(U&& v)
{
	static_assert(std::is_same<T, std::remove_reference<U>::type>::value, "U must be implicitly convertible to T.");

	{
		std::lock_guard<std::mutex> lock(mutex_);
		bool res = queue_.try_push(std::forward<U>(v));
		assert(res);
	}

	not_empty_condition_.notify_one();
}

template<typename T>
template<typename InputIt>
void concurrent_queue<T>::push(InputIt b, InputIt e)
{
	using trait = std::iterator_traits<InputIt>;
	static_assert(std::is_same<T, trait::value_type>::value, "InputIt::value_type must be implicitly convertible to T.");
	
	{
		std::lock_guard<std::mutex> lock(mutex_);
		for (InputIt i = b; i != e; ++i) {
			bool res = queue_.try_push(std::forward<trait::reference>(*i));
			assert(res);
		}
	}

	not_empty_condition_.notify_all();
}

template<typename T>
bool concurrent_queue<T>::try_pop(T& out_v)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (queue_.empty()) return false;

	return queue_.try_pop(out_v);
}

template<typename T>
bool concurrent_queue<T>::wait_pop(T& out_v)
{
	// We wait while the queue_ is empty and waiting is allowed.
	std::unique_lock<std::mutex> lock(mutex_);
	not_empty_condition_.wait(lock, [this] { return !(queue_.empty() && wait_allowed_); });

	if (queue_.empty()) return false;

	return queue_.try_pop(out_v);
}

} // namespace ts

#endif // TS_CONCURRENCY_QUEUE_H_
