#include "ts/fiber.h"

#include <cassert>
#include <algorithm>
#include <windows.h>


namespace ts {

// ----- fiber -----

fiber::fiber(fiber_func_t func, size_t stack_byte_count, void* p_data)
{
	assert(func);
	assert(stack_byte_count > 0);
	p_handle = CreateFiber(stack_byte_count, func, p_data);
}

fiber::fiber(fiber&& fbr) noexcept
	: p_handle(fbr.p_handle)
{
	fbr.p_handle = nullptr;
}

fiber& fiber::operator=(fiber&& fbr) noexcept
{
	if (this == &fbr) return *this;

	dispose();
	p_handle = fbr.p_handle;
	fbr.p_handle = nullptr;

	return *this;
}

fiber::~fiber() noexcept
{
	dispose();
}

void fiber::dispose() noexcept
{
	if (!p_handle) return;
	DeleteFiber(p_handle);
	p_handle = nullptr;
}

// ----- fiber_pool -----

fiber_pool::fiber_pool(size_t fiber_count, void(*func)(void*), size_t stack_byte_count, void* p_data)
{
	assert(fiber_count > 0);
	assert(func);
	assert(stack_byte_count);

	fibers_.resize(fiber_count);
	for (auto& e : fibers_)
		e.fiber = fiber(func, stack_byte_count, p_data);

	std::sort(fibers_.begin(), fibers_.end(), 
		[](const list_entry& l, const list_entry& r) { return l.fiber.p_handle < r.fiber.p_handle; });
}

void fiber_pool::push_back(void* p_fbr)
{
	using it_t = decltype(fibers_)::iterator;

	assert(p_fbr);
	std::lock_guard<std::mutex> lock(mutex_);

	it_t it = std::lower_bound(fibers_.begin(), fibers_.end(), p_fbr,
		[](const list_entry& e, const void* p_fbr)  { return (e.fiber.p_handle < p_fbr); });
	assert(it != fibers_.end());
	
	it->in_use = false;
}

void* fiber_pool::pop()
{
	using it_t = decltype(fibers_)::iterator;

	std::lock_guard<std::mutex> lock(mutex_);
	it_t it = std::find_if(fibers_.begin(), fibers_.end(), [](const list_entry& e) { return !e.in_use; });
	
	if (it == fibers_.end()) {
		return nullptr;
	}
	else {
		it->in_use = true;
		return it->fiber.p_handle;
	}
}

// ----- fiber_wait_list -----

fiber_wait_list::fiber_wait_list(size_t fiber_count)
{
	wait_list_.resize(fiber_count);
}

void fiber_wait_list::push(void* p_fiber, const std::atomic_size_t* p_wait_counter)
{
	assert(p_fiber);
	assert(p_wait_counter);
	assert(*p_wait_counter > 0);

	std::lock_guard<std::mutex> lock(mutex_);
	assert(push_index_ < wait_list_.size());

	wait_list_[push_index_] = list_entry{ p_fiber, p_wait_counter };
	++push_index_;
}

bool fiber_wait_list::try_pop(void*& p_out_fiber)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (push_index_ == 0) return false;

	for (size_t i = push_index_; i > 0; --i) {
		list_entry& e = wait_list_[i - 1];
		if (*e.p_wait_counter > 0) continue;

		// remove i - 1 fiber from the list
		p_out_fiber = e.p_fiber;
		e.p_fiber = nullptr;
		e.p_wait_counter = nullptr;

		if (i != push_index_)
			std::swap(e, wait_list_[push_index_ - 1]);

		--push_index_;
		return true;
	}

	return false;
}

// ----- thread_fiber_nature -----

thread_fiber_nature::thread_fiber_nature()
	: p_handle(ConvertThreadToFiber(GetCurrentThread()))
{
	assert(p_handle);
	assert(GetLastError() != ERROR_ALREADY_FIBER);
}

thread_fiber_nature::~thread_fiber_nature()
{
	ConvertFiberToThread();
}

// ----- funcs -----

std::ostream& operator<<(std::ostream& o, const fiber& f)
{
	o << "fiber(" << f.p_handle << ')';
	return o;
}

std::wostream& operator<<(std::wostream& o, const fiber& f)
{
	o << "fiber(" << f.p_handle << ')';
	return o;
}

void* current_fiber()
{
	return GetCurrentFiber();
}

void* fiber_data()
{
	return GetFiberData();
}

void switch_to_fiber(void* p_fbr) noexcept
{
	assert(p_fbr);
	SwitchToFiber(p_fbr);
}

} // namespace ts
