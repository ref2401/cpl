#ifndef TS_FIBER_H_
#define TS_FIBER_H_


namespace ts {

class fiber final {

	fiber() noexcept = default;

	fiber(void(*func)(void*), size_t stack_byte_limit, void* data = nullptr);

	fiber(fiber&& fbr) noexcept;
	fiber& operator=(fiber&& fbr) noexcept;

	~fiber() noexcept;


	void dispose() noexcept;

	void* p_sys_obj = nullptr;
};

} // namespace ts

#endif // TS_FIBER_H_
