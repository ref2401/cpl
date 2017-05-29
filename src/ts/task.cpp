#include "ts/task.h"

#include <cassert>
#include <memory>


namespace {

using namespace ts;

class task_system;

std::unique_ptr<task_system> g_task_system;


class task_system final {
public:

	explicit task_system(const task_system_desc& desc) {}

	task_system(const task_system&) = delete;

	task_system(task_system&&) = delete;

	~task_system() noexcept {}


	task_system& operator=(const task_system&) = delete;

	task_system& operator=(task_system&&) = delete;


	void terminate() {}

private:


};

} // namespace


namespace ts {

void init_task_system(const task_system_desc& desc)
{
	assert(!g_task_system);
	g_task_system = std::make_unique<task_system>(desc);
}

void terminate_task_system()
{
	if (!g_task_system) return;

	g_task_system->terminate();
	g_task_system.reset();
}

} // namespace ts
