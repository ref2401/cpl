# cpl

## design
Thread {
	std::thread _thread;
	Fiber _{controling/head/main/}_fiber;
};

Task_system_desc {
	thread_count
	fiber_count
};

Task_queue (multiple?) owns tasks
Task_system runs tasks using fibers from fiber pool
Fiber_pool

## steps
- concurrent_queue, ABA problem.
- task_system, run_tasks
- fork-join (task_counter)
- fibers
- more patterns from structured parallel programming
- task_system terminates at any moment. ?silently swallows? exceptions if any.
 

## Bibliography
- [Structured Parallel Programming: Patterns for Efficient Computation](https://www.amazon.com/Structured-Parallel-Programming-Efficient-Computation/dp/0124159931/ref=sr_1_1?ie=UTF8&qid=1491320996&sr=8-1&keywords=structured+parallel+programming)
- [The Art of Multiprocessor Programming](https://www.amazon.com/Art-Multiprocessor-Programming-Revised-Reprint/dp/0123973376/ref=sr_1_2?ie=UTF8&qid=1491320996&sr=8-2&keywords=structured+parallel+programming)
- [TBB](https://www.threadingbuildingblocks.org/)
- [HPX](https://github.com/STEllAR-GROUP/hpx)
- [GDC2014 talk: Parallelizing the Naughty Dog Engine Using Fibers](http://www.gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine)

- https://en.wikipedia.org/wiki/Fiber_(computer_science)
- Portable Coroutine Library http://freecode.com/projects/libpcl/
- CFiber http://www.flounder.com/fibers.htm

```c++

void do_something();
void do_something_else(int);

void main() {
	task_system ts;

	task_counter tc_0 = run_task(do_something);
	wait_for(tc_0, 1);

	task_desc td(do_something_else, 24);
	task_counter tc_1 = run_task(td);
	wait_for(tc_1, 1);
}


```
