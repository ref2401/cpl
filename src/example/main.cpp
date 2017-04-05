#include <iostream>
#include "cpl/task.h"


void do_something()
{
	int k = 0;
	k += 2;
}

void main(int argc, char* argv[])
{
	auto lb = []() {
		int k = 0;
		++k;
	};

	std::function<void()> func = []() {
		int k = 0;
		k += 24;
	};

	cpl::task tasks[3] = {
		cpl::task(lb),
		cpl::task(func),
		cpl::task(do_something)
	};

	cpl::run_tasks(tasks);
}