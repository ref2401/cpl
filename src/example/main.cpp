#include <atomic>
#include <iostream>
#include <mutex>
#include <type_traits>
#include "ts/task.h"


void do_something()
{
	int k = 0;
	k += 2;
}


void main(int argc, char* argv[])
{
	std::cout << std::is_move_constructible<int>::value << std::endl;
	std::cout << std::is_move_assignable<std::atomic_bool>::value << std::endl;

	auto lb = []() {
		int k = 0;
		++k;
	};

	std::function<void()> func = []() {
		int k = 0;
		k += 24;
	};

	ts::task tasks[3] = {
		ts::task(lb),
		ts::task(func),
		ts::task(do_something)
	};

	ts::run_tasks(tasks);
}