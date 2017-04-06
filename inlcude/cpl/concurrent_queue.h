#ifndef CPL_CONCURRENCY_QUEUE_H_
#define CPL_CONCURRENCY_QUEUE_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <type_traits>


namespace cpl {

template<typename T>
class concurrent_queue final {
public:

	concurrent_queue() noexcept
		: _wait_allowed(true)
	{}


	//void clear();

	bool empty() const;

	size_t size() const;

	void lock() const 
	{ 
		_mutex.lock(); 
	}

	void unlock() const 
	{
		_mutex.unlock();
	}

	template<typename U>
	void push(U&& v);

	bool try_pop(T& out_v);

	bool wait_pop(T& out_v);

private:

	std::queue<T> _queue;
	mutable std::mutex _mutex;
	std::condition_variable _not_empty_condition;
	std::atomic_bool _wait_allowed;
};


//template<typename T>
//void concurrent_queue<T>::clear()
//{
//	std::lock_guard<std::mutex> lock(_mutex);
//	
//}

template<typename T>
bool concurrent_queue<T>::empty() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _queue.empty();
}

template<typename T>
template<typename U>
void concurrent_queue<T>::push(U&& v)
{
	static_assert(std::is_same<T, std::remove_reference<U>::type>::value, "U must be implicitly convertible to T.");

	std::lock_guard<std::mutex> lock(_mutex);
	_queue.push(std::forward<U>(v));
	_not_empty_condition.notify_one();
}

template<typename T>
size_t concurrent_queue<T>::size() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _queue.size();
}

template<typename T>
bool concurrent_queue<T>::try_pop(T& out_v)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_queue.empty()) return false;

	out_v = std::move(_queue.front());
	_queue.pop();

	return true;
}

template<typename T>
bool concurrent_queue<T>::wait_pop(T& out_v)
{
	std::unique_lock<std::mutex> lock(_mutex);
	_not_empty_condition.wait(lock, [this] { return !_queue.empty(); });

	out_v = std::move(_queue.front());
	_queue.pop();

	return true;
}

} // namespace cpl

#endif // CPL_CONCURRENCY_QUEUE_H_
