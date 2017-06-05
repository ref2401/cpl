#include <iostream>
#include <atomic>
#include "ts/task_system.h"
#include "example/example.h"


void do_something(int a)
{
	int k = a;
	k += 2;
}

void main(int argc, char* argv[])
{
	std::atomic_size_t wc = 10;
	std::atomic_size_t* p_wc = &wc;
	std::cout << (p_wc > 100) << std::endl;

	ts::task_system_desc desc;
	desc.high_queue_size = 8;
	desc.queue_size = 128;
	desc.thread_count = 4;
	ts::init_task_system(desc);

	example::map_example();

	auto report = ts::terminate_task_system();
	std::cout << "high_task_count: " << report.high_task_count << std::endl;
	std::cout << "task_count: " << report.task_count << std::endl;
	std::cin.get();
}