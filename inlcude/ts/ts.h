#ifndef TS_TS_H_
#define TS_TS_H_

#include <cassert>
#include <functional>
#include <type_traits>
#include <utility>


namespace ts {

struct task_desc final {
	task_desc() noexcept = default;

	template<typename F, typename... Args>
	explicit task_desc(F&& f, Args&&... args)
		: func(std::bind(std::forward<F>(f), std::forward<Args>(args)...))
	{}
	
	std::function<void()> func;
};

struct task_system_desc final {
	size_t thread_count = 0;
	size_t high_queue_size = 0;
	size_t queue_size = 0;
};

struct task_system_report final {
	// The number of processed tasks with high priority.
	size_t high_task_count = 0;

	// The number of processed tasks.
	size_t task_count = 0;
};


void init_task_system(const task_system_desc& desc);

task_system_report terminate_task_system();

void run(task_desc* p_tasks, size_t count);

template<size_t count>
inline void run(task_desc(&tasks)[count])
{
	static_assert(count > 0, "The number of tasks must be >= 1.");
	run(tasks, count);
}

// How many worker threads are used by the current task system.
// The value is equal to task_system_desc.thread_count
size_t thread_count() noexcept;

} // namespace ts

#endif // TS_TS_H_
