# cpl

## steps
- fiber
- 2 fibers cout << "something". test
- wait_for
- concurrent_queue, ABA problem.
- map reduce
- fork-join (task_counter)
- more patterns from structured parallel programming
 

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

struct fiber final {};

class fiber_thread final {};


template<typename T>
T get_current_fiber_data();

void* get_current_fiber_data();



struct task_system_context {
	queue_high;
	queue;
	wait_list;
	exec_flag;
};


void main_fiber_func(task_system_context ctx)
{
	fiber fbr;
	ctx.fiber_pool.pop(fbr);
	assert(has_system_object(fbr));
	call(fbr);
}

void worker_fiber_func(void* data)
{
	// ? How exactly do I get the context ?
	auto& ctx = get_task_system_context(data);

	while (ctx.exec_flag) {
		fiber fbr = find_available_fiber(ctx.wait_list);

		if (has_system_object(fbr)) {
			
		}

		task t;
		bool res = ctx.queue.pop(t);
		if (res) 
			t.func();
	}
}

void wait_for(exec_object exec_obj)
{
	auto& ctx = get_worker_context();
	ctx.fiber_wait_list.push(ctx.curr_fiber, exec_obj);
	switch_to_fiber(ctx.main_fiber);
}

```