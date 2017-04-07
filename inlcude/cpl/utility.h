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
		return (curr_count == 0);
	}

	size_t size() const noexcept
	{
		return curr_count;
	}

	size_t size_limit() const noexcept 
	{
		return buffer.size();
	}


	template<typename U>
	bool try_push(U&& v);

	bool try_pop(T& out_v);

private:

	std::vector<T> buffer;
	size_t push_index = 0;
	size_t pop_index = 0;
	size_t curr_count = 0;
};

template<typename T>
ring_buffer<T>::ring_buffer(size_t size_limit)
	: buffer(size_limit)
{
}

template<typename T>
ring_buffer<T>::ring_buffer(ring_buffer<T>&& rb) noexcept
	: buffer(std::move(rb.buffer)),
	push_index(rb.push_index),
	pop_index(rb.pop_index),
	curr_count(rb.curr_count)
{
	rb.push_index = 0;
	rb.pop_index = 0;
	rb.curr_count = 0;
}

template<typename T>
ring_buffer<T>& ring_buffer<T>::operator=(ring_buffer<T>&& rb) noexcept
{
	if (this == &rb) return *this;

	buffer = std::move(rb.buffer);
	push_index = rb.push_index;
	pop_index = rb.pop_index;
	curr_count = rb.curr_count;

	rb.push_index = 0;
	rb.pop_index = 0;
	rb.curr_count = 0;

	return *this;
}

template<typename T>
template<typename U>
bool ring_buffer<T>::try_push(U&& v)
{
	if (curr_count == buffer.size()) return false;

	buffer[push_index % buffer.size()] = std::forward<U>(v);
	++push_index;
	++curr_count;

	return true;
}

template<typename T>
bool ring_buffer<T>::try_pop(T& out_v)
{
	if (curr_count == 0) return false;

	out_v = std::move(buffer[pop_index % buffer.size()]);
	++pop_index;
	--curr_count;

	return true;
}

} // namespace

#endif // CPL_UTILITY_H_
