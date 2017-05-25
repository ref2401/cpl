#ifndef CPL_UTILITY_H_
#define CPL_UTILITY_H_

#include <cassert>
#include <vector>


namespace cpl {

template<typename T>
class ring_buffer final {
public:

	ring_buffer() noexcept = default;

	explicit ring_buffer(size_t size_limit);

	ring_buffer(const ring_buffer<T>&) = default;

	ring_buffer(ring_buffer<T>&& rb) noexcept;


	ring_buffer<T>& operator=(const ring_buffer<T>&) = default;

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
{
}

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
template<typename U>
bool ring_buffer<T>::try_push(U&& v)
{
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

} // namespace

#endif // CPL_UTILITY_H_
