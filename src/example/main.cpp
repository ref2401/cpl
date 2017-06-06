#include <iostream>
#include <atomic>
#include "ts/task_system.h"
#include "example/example.h"

#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
#include <windows.h>

constexpr size_t thread_count = 2;
void* fiber_handles[thread_count];
thread_local void* gp_main_thread_fiber;
thread_local size_t g_var;

std::mutex mutex_cout;

void fiber_func(void*)
{
	for (size_t i = 0; i < thread_count; ++i) {
		{
			std::lock_guard<std::mutex> lock(mutex_cout);
			std::cout << "------ " << GetCurrentFiber() << "------ " << std::endl;
			std::cout << "fiber_func: " << g_var << std::endl;
		}

		SwitchToFiber(gp_main_thread_fiber);
	}
}

void thread_func(size_t thread_index)
{
	assert(thread_index < thread_count);
	
	// give the thread fiber nature
	void* p_thread_fiber = ConvertThreadToFiber(GetCurrentThread());
	gp_main_thread_fiber = p_thread_fiber;
	g_var = thread_index;

	// create a fiber and exec it
	fiber_handles[thread_index] = CreateFiber(128, fiber_func, nullptr);
	SwitchToFiber(fiber_handles[thread_index]);

	// exec the next fiber in the list
	std::this_thread::sleep_for(std::chrono::milliseconds(250)); // terrible hack to make sure the fiber is not being run anymore by another thread
	const size_t idx = (thread_index + 1) % thread_count;
	SwitchToFiber(fiber_handles[idx]);

	// clean up
	std::this_thread::sleep_for(std::chrono::milliseconds(250)); // terrible hack to make sure the fiber is not being run anymore by another thread
	DeleteFiber(fiber_handles[thread_index]);
	fiber_handles[thread_index] = nullptr;
	ConvertFiberToThread();
}

void main(int argc, char* argv[])
{
	std::vector<std::thread> thread_list;
	thread_list.reserve(thread_count);

	for (size_t i = 0; i < thread_count; ++i)
		thread_list.emplace_back(thread_func, i);

	for (auto& th : thread_list)
		th.join();

	//ts::task_system_desc desc;
	//desc.high_queue_size = 8;
	//desc.queue_size = 128;
	//desc.thread_count = 4;
	//ts::init_task_system(desc);

	//example::map_example();

	//auto report = ts::terminate_task_system();
	//std::cout << "high_task_count: " << report.high_task_count << std::endl;
	//std::cout << "task_count: " << report.task_count << std::endl;
	std::cin.get();
}