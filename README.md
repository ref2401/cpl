# cpl

## steps
- concurrent_queue, ABA problem.
- wait_for
- task_system, run_tasks
- map reduce
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
void process_objects(it begin, it end);

void main() {
	init_task_system();
	finalize_task_system();

	// standalone tasks
	task t0(do_something);
	run_task(t0);

	task t1(do_something_else, 24);
	run_task(t1);

	wait_for(t0, t1);
	// assert if any task has not been run;


	// map pattern (with tiling)
	task task_array[tile_count];
	object object_array[object_count] = { ... };
	size_t tile_object_count = object_count / tile_count;
	
	for [0, tile_count) {
		it begin = object_array + i * tile_object_count;
		it end = begin + tile_object_count;
		task_array[i] = task(process_objects, begin, end);
	}

	wait_for(task_array);
}


```
