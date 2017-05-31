#include "ts/fiber.h"

#include <cassert>
#include <algorithm>
#include <windows.h>


namespace ts {

// ----- fiber -----

fiber::fiber(void(*func)(void*), size_t stack_byte_limit, void* data = nullptr)
{
	assert(func);
	assert(stack_byte_limit > 0);

	p_sys_obj = CreateFiber(stack_byte_limit, func, data);
	assert(p_sys_obj);
}

fiber::fiber(fiber&& fbr) noexcept
	: p_sys_obj(fbr.p_sys_obj)
{
	fbr.p_sys_obj = nullptr;
}

fiber& fiber::operator=(fiber&& fbr) noexcept
{
	if (this == &fbr) return *this;

	dispose();

	p_sys_obj = fbr.p_sys_obj;

	fbr.p_sys_obj = nullptr;

	return *this;
}

fiber::~fiber() noexcept
{
	dispose();
}

void fiber::dispose() noexcept
{
	if (!p_sys_obj) return;

	DeleteFiber(p_sys_obj);
	p_sys_obj = nullptr;
}

} // namespace ts