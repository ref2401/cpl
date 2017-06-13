#ifndef TS_TS_H_
#define TS_TS_H_

#include <cassert>
#include <functional>
#include <type_traits>
#include <utility>
#include "ts/task.h"


namespace ts {

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
