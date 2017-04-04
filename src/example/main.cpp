#include <iostream>
#include "tpl/task.h"


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

	tpl::task tasks[3] = {
		tpl::task(lb),
		tpl::task(func),
		tpl::task(do_something)
	};

	tpl::run_tasks(tasks);
}