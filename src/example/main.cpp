#include <iostream>
#include <atomic>
#include "ts/task_system.h"
#include "example/example.h"

#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
#include <windows.h>


void run_examples(std::atomic_bool& exec_flag)
{
	using namespace example;

	std::atomic_size_t wait_counter;
	ts::run(simple_map_example, wait_counter);
	ts::wait_for(wait_counter);

	exec_flag = false;
}

void main(int argc, char* argv[])
{
	ts::task_system_desc ts_desc = {
		/* thread_count */				1,
		/* fiber_count */				8,
		/* fiber_stack_byte_count */	128,
		/* queue_size */				64,
		/* queue_immediate_size */		16
	};
	
	auto report = ts::launch_task_system(ts_desc, run_examples);

	std::cout << std::endl << "[task system report]" << std::endl
		<< "\thigh_task_count: " << report.high_task_count << ";" << std::endl
		<< "\ttask_count: " << report.task_count << ";" << std::endl;
	std::cin.get();
}