#ifndef TS_FIBER_H_
#define TS_FIBER_H_

#include <thread>


namespace ts {

struct fiber final {

	fiber() noexcept = default;

	fiber(void(*func)(void*), size_t stack_byte_limit, void* data = nullptr);

	fiber(fiber&& fbr) noexcept;
	fiber& operator=(fiber&& fbr) noexcept;

	~fiber() noexcept;


	void dispose() noexcept;

	void* p_sys_obj = nullptr;
};

//class thread_main_fiber final {
//public:
//
//	thread_main_fiber(std::thread& t);
//
//private:
//
//	void* p_handle_ = nullptr;
//};

} // namespace ts

#endif // TS_FIBER_H_
