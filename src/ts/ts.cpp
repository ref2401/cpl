#include "ts/ts.h"

#include <cassert>
#include <limits>
#include <memory>
#include <vector>
#include <thread>
#include <ts/concurrent_queue.h>


namespace {

using namespace ts;


struct task final {
	std::function<void()> func;
};

struct task_system_context final {

	task_system_context(size_t queue_high_size, size_t queue_size)
		: queue_high(queue_high_size), queue(queue_size)
	{
		assert(queue_high_size > 0);
		assert(queue_size > 0);
	}

	concurrent_queue<task> queue_high;
	concurrent_queue<task> queue;
	std::vector<std::thread> worker_threads;
	task_system_report report;
	std::atomic_bool exec_flag;
};


void worker_thread_func(const std::atomic_bool& exec_flag, concurrent_queue<task>& queue_high, concurrent_queue<task>& queue_medium)
{
	while (exec_flag) {
		// drain the queue_high
		while (exec_flag && !queue_high.empty()) {
			task t;
			bool pop_res = queue_high.try_pop(t);
			if (pop_res) t.func();
		}

		// get one task from the queue_medium
		task t;
		bool pop_res = queue_medium.try_pop(t);
		if (pop_res) t.func();
	}
}

} // namespace


namespace ts {

void init_task_system(const task_system_desc& desc)
{
	//assert(!gp_ts_ctx);

	//gp_ts_ctx = std::make_unique<task_system_context>(desc.high_queue_size, desc.queue_size);

	//gp_ts_ctx->exec_flag = true;
	//gp_ts_ctx->worker_threads.reserve(desc.thread_count);
	//for (size_t i = 0; i < desc.thread_count; ++i) {
	//	gp_ts_ctx->worker_threads.emplace_back(worker_thread_func,
	//		std::ref(gp_ts_ctx->exec_flag),
	//		std::ref(gp_ts_ctx->queue_high),
	//		std::ref(gp_ts_ctx->queue));
	//}
}

task_system_report terminate_task_system()
{
	//if (!gp_ts_ctx) return { };

	//gp_ts_ctx->exec_flag = false;
	//gp_ts_ctx->queue_high.set_wait_allowed(false);
	//gp_ts_ctx->queue.set_wait_allowed(false);

	//for (auto& th : gp_ts_ctx->worker_threads)
	//	th.join();

	//task_system_report report = gp_ts_ctx->report;
	//gp_ts_ctx.reset();

	//return report;
	return {};
}

void run(task_desc* p_tasks, size_t count)
{
	//assert(p_tasks);
	//assert(count > 0);

	//gp_ts_ctx->report.task_count += count;

	//for (size_t i = 0; i < count; ++i) {
	//	auto& td = p_tasks[i];
	//	gp_ts_ctx->queue.push(task{ std::move(td.func) });
	//}
}

size_t thread_count() noexcept
{
	//return gp_ts_ctx->worker_threads.size();
	return 0;
}

} // namespace ts
