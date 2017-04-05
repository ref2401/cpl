#ifndef CPL_TASK_H_
#define CPL_TASK_H_

#include <functional>
#include <type_traits>
#include <utility>


namespace cpl {

struct task final {

	task() noexcept = default;

	template<typename F>
	explicit task(F&& f)
		: func(std::forward<F>(f))
	{}


	std::function<void()> func;
};

struct task_system_desc final {
};



void init_task_system(const task_system_desc& desc);

void terminate_task_system();

template<size_t count>
void run_tasks(const task(&tasks)[count])
{
	static_assert(count > 0, "The number of tasks must be >= 1.");
	for (size_t i = 0; i < count; ++i)
		tasks[i].func();
}

} // namespace cpl

#endif // CPL_TASK_H_
