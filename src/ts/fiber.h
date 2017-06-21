#ifndef TS_FIBER_H_
#define TS_FIBER_H_

#include <iostream>
#include <mutex>
#include <vector>


namespace ts {

using fiber_func_t = void(*)(void*);

struct fiber final {
	fiber() noexcept = default;

	fiber(fiber_func_t func, size_t stack_byte_count, void* p_data = nullptr);

	fiber(fiber&& fbr) noexcept;
	fiber& operator=(fiber&& fbr) noexcept;

	~fiber() noexcept;


	void dispose() noexcept;


	void* p_handle = nullptr;
};

// The only source of fiber objects.
class fiber_pool final {
public:

	fiber_pool(size_t fiber_count, void (*func)(void*), size_t stack_byte_count, void* p_data = nullptr);

	fiber_pool(fiber_pool&&) = delete;
	fiber_pool& operator=(fiber_pool&&) = delete;


	// Puts back the specified fiber object for later reuse.
	// asserts if the specified pointer is not a fiber from the pool.
	void push_back(void* p_fbr);

	// Returns a pointer to a fiber object or nullptr if there are no fibers left.
	void* pop();


private:

	struct list_entry final {
		fiber fiber;
		bool in_use = false;
	};


	std::vector<list_entry> fibers_;
	std::mutex mutex_;
};

// thread_fiber_nature object makes it possible to execute fibers inside the current thread.
// Usually you want to create an instance at the beginign of the thread's function and forget about it.
// It is illegal to create several objects in the same thread!
struct thread_fiber_nature final {
	thread_fiber_nature();

	thread_fiber_nature(thread_fiber_nature&&) = delete;
	thread_fiber_nature& operator=(thread_fiber_nature&&) = delete;

	~thread_fiber_nature();


	void* p_handle;
};


void* current_fiber();

void* fiber_data();

template<typename T>
inline T* fiber_data()
{
	return static_cast<T*>(fiber_data());
}

void switch_to_fiber(void* p_fbr) noexcept;

} // namespace ts

#endif // TS_FIBER_H_
