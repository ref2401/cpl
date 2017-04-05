#ifndef CPL_CONCURRENCY_QUEUE_H_
#define CPL_CONCURRENCY_QUEUE_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>


template<typename T>
class concurrency_queue final {
public:

	concurrency_queue() noexcept
		: _wait_allowed(true)
	{}


	//void clear();

	bool empty() const;

	void push(T&& v);

	size_t size() const;

	bool try_pop(T& out_v);

	bool wait_pop(T& out_v);

private:

	std::queue<T> _queue;
	std::mutex _mutex;
	std::condition_variable _not_empty_condition;
	std::atomic_bool _wait_allowed;
};


//template<typename T>
//void concurrency_queue<T>::clear()
//{
//	std::lock_guard<std::mutex> lock(_mutex);
//	
//}

template<typename T>
bool concurrency_queue<T>::empty() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _queue.empty();
}

template<typename T>
void concurrency_queue<T>::push(T&& v)
{
	std::lock_quard<std::mutex> lock(_mutex);
	_queue.push(std::forward<T>(v));
	_not_empty_condition.notify_one();
}

template<typename T>
size_t concurrency_queue<T>::size() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _queue.size();
}

template<typename T>
bool concurrency_queue<T>::try_pop(T& out_v)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_queue.empty()) return false;

	out_v = std::move(_queue.front());
	_queue.pop();

	return true;
}

template<typename T>
bool concurrency_queue<T>::wait_pop(T& out_v)
{
	std::unique_lock<std::mutex> lock(_mutex);
	_not_empty_condition.wait(lock, [this] { return !_queue.empty(); });

	out_v = std::move(_queue.front());
	_queue.pop();

	return true;
}

#endif // CPL_CONCURRENCY_QUEUE_H_
