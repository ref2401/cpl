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
		e.p_fiber = CreateFiber(stack_byte_count, func, p_data);

	std::sort(fibers_.begin(), fibers_.end(), fiber_pool::list_entry_comparer);
}

fiber_pool::~fiber_pool() noexcept
{
	for (auto& e : fibers_) {
		DeleteFiber(e.p_fiber);
		e.p_fiber = nullptr;
	}
}

void fiber_pool::push_back(void* p_fbr)
{
	using it_t = decltype(fibers_)::iterator;

	assert(p_fbr);
	std::lock_guard<std::mutex> lock(mutex_);

	const list_entry entry = { p_fbr, false };
	it_t it = std::lower_bound(fibers_.begin(), fibers_.end(), entry, fiber_pool::list_entry_comparer);
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
		return it->p_fiber;
	}
}

// ----- thread_main_fiber -----

thread_main_fiber::thread_main_fiber()
	: p_fiber(ConvertThreadToFiber(GetCurrentThread()))
{
	assert(p_fiber);
	assert(GetLastError() != ERROR_ALREADY_FIBER);
}

thread_main_fiber::~thread_main_fiber()
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
