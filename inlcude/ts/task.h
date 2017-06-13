#ifndef TS_TASK_H_
#define TS_TASK_H_




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
	size_t fiber_count = 0;
	size_t queue_size = 0;
	size_t queue_immediate_size = 0;
};

struct task_system_report final {
	// The number of processed tasks with high priority.
	size_t high_task_count = 0;

	// The number of processed tasks.
	size_t task_count = 0;
};

} // namespace

#endif // TS_TASK_H_
