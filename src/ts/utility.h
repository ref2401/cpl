#ifndef TS_UTILITY_H_
#define TS_UTILITY_H_

#include <cassert>
#include <mutex>
#include <type_traits>
#include <vector>


namespace ts {

class exception_slot final {
public:

	exception_slot() = default;

	exception_slot(exception_slot&&) noexcept = default;
	exception_slot& operator=(exception_slot&&) noexcept = default;


	bool has_exception() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return (exception_ != nullptr);
	}

	std::exception_ptr exception() const noexcept
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return exception_;
	}

	void set_exception(std::exception_ptr e) noexcept
	{
		std::lock_guard<std::mutex> lock(mutex_);
		exception_ = e;
	}

private:

	std::exception_ptr	exception_;
	mutable std::mutex	mutex_;
};

template<typename T>
class ring_buffer final {
public:

	ring_buffer() noexcept = default;

	explicit ring_buffer(size_t size_limit);

	ring_buffer(const ring_buffer<T>&) = default;
	ring_buffer<T>& operator=(const ring_buffer<T>&) = default;

	ring_buffer(ring_buffer<T>&& rb) noexcept;
	ring_buffer<T>& operator=(ring_buffer<T>&& rb) noexcept;


	bool empty() const noexcept
	{
		return (curr_count_ == 0);
	}

	size_t size() const noexcept
	{
		return curr_count_;
	}

	size_t size_limit() const noexcept 
	{
		return buffer_.size();
	}


	template<typename... Args>
	bool try_emplace(Args&&... args);

	template<typename U>
	bool try_push(U&& v);

	bool try_pop(T& out_v);

private:

	std::vector<T> buffer_;
	size_t push_index_ = 0;
	size_t pop_index_ = 0;
	size_t curr_count_ = 0;
};

template<typename T>
ring_buffer<T>::ring_buffer(size_t size_limit)
	: buffer_(size_limit)
{}

template<typename T>
ring_buffer<T>::ring_buffer(ring_buffer<T>&& rb) noexcept
	: buffer_(std::move(rb.buffer_)),
	push_index_(rb.push_index_),
	pop_index_(rb.pop_index_),
	curr_count_(rb.curr_count_)
{
	rb.push_index_ = 0;
	rb.pop_index_ = 0;
	rb.curr_count_ = 0;
}

template<typename T>
ring_buffer<T>& ring_buffer<T>::operator=(ring_buffer<T>&& rb) noexcept
{
	if (this == &rb) return *this;

	buffer_ = std::move(rb.buffer_);
	push_index_ = rb.push_index_;
	pop_index_ = rb.pop_index_;
	curr_count_ = rb.curr_count_;

	rb.push_index_ = 0;
	rb.pop_index_ = 0;
	rb.curr_count_ = 0;

	return *this;
}

template<typename T>
template<typename... Args>
bool ring_buffer<T>::try_emplace(Args&&... args)
{
	return try_push(T { std::forward<Args>(args)... });
}

template<typename T>
template<typename U>
bool ring_buffer<T>::try_push(U&& v)
{
	static_assert(std::is_same<T, std::remove_reference<U>::type>::value, "U must be implicitly convertible to T.");

	if (curr_count_ == buffer_.size()) return false;

	buffer_[push_index_ % buffer_.size()] = std::forward<U>(v);
	++push_index_;
	++curr_count_;

	return true;
}

template<typename T>
bool ring_buffer<T>::try_pop(T& out_v)
{
	if (curr_count_ == 0) return false;

	out_v = std::move(buffer_[pop_index_ % buffer_.size()]);
	++pop_index_;
	--curr_count_;

	return true;
}

} // namespace ts

#endif // TS_UTILITY_H_
