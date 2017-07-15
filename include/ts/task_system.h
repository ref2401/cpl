#ifndef TS_TS_H_
#define TS_TS_H_

// README (ugly version):
// Kernel thread is the thread which calls ts::launch_task_system.
// Kernel function is the function passed to ts::launch_task_system. These function is always
// performed inside the kernel thread.
// Each worker thread has a controller fiber. The controller fiber dispathes tasks (inside other fibers)
// ...
// The kernel thread has the controller fiber and an addition fiber (kernel fiber) which is used to run the kernel func.
// The kernel therad never puts the kernel fiber into the fiber pool or fiber wait list.

#include <cassert>
#include <atomic>
#include <functional>
#include <type_traits>
#include <utility>


namespace ts {

using kernel_func_t = void(*)();

struct task_system_desc final {
	size_t thread_count = 0;
	size_t fiber_count = 0;
	size_t fiber_stack_byte_count = 0;
	size_t queue_size = 0;
	size_t queue_immediate_size = 0;
};

struct task_system_report final {
	// The number of processed tasks with high priority.
	size_t task_immediate_count = 0;

	// The number of processed tasks.
	size_t task_count = 0;
};


inline bool is_valid_task_system_desc(const task_system_desc& desc) noexcept
{
	return (desc.thread_count > 0)
		&& (desc.fiber_count > 0)
		&& (desc.queue_size > 0)
		&& (desc.queue_immediate_size > 0);
}

task_system_report launch_task_system(const task_system_desc& desc, kernel_func_t p_kernel_func);

// How many worker threads are used by the current task system.
// The value is equal to task_system_desc.thread_count
size_t thread_count() noexcept;

void wait_for(const std::atomic_size_t& wait_counter);

void run(std::atomic_size_t* p_wait_counter, std::function<void()>* p_funcs, size_t count);

inline void run(std::atomic_size_t& wait_counter, std::function<void()>& func)
{
	run(&wait_counter, &func, 1);
}

inline void run(std::function<void()>& func)
{
	run(nullptr, &func, 1);
}

template<size_t count>
inline void run(std::atomic_size_t& wait_counter, std::function<void()>(&funcs)[count])
{
	run(&wait_counter, funcs, count);
}

template<size_t count>
inline void run(std::function<void()>(&funcs)[count])
{
	run(nullptr, funcs, count);
}

inline void run(std::atomic_size_t& wait_counter, void(*func)())
{
	std::function<void()> f(func);
	run(&wait_counter, &f, 1);
}

inline void run(void(*func)())
{
	std::function<void()> f(func);
	run(nullptr, &f, 1);
}

} // namespace ts

#endif // TS_TS_H_
