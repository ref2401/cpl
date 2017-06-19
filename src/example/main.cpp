#include <iostream>
#include <atomic>
#include "ts/task_system.h"
#include "example/example.h"

#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
#include <windows.h>


void main(int argc, char* argv[])
{
	ts::task_system_desc ts_desc = {
		/* thread_count */				2,
		/* fiber_count */				8,
		/* fiber_stack_byte_count */	128,
		/* queue_size */				64,
		/* queue_immediate_size */		16
	};
	ts::init_task_system(ts_desc);

	example::run_examples();

	auto report = ts::terminate_task_system();
	std::cout << std::endl << "[task system report]" << std::endl
		<< "\thigh_task_count: " << report.high_task_count << ";" << std::endl
		<< "\ttask_count: " << report.task_count << ";" << std::endl;
	std::cin.get();
}