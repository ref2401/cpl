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

struct task_system final {
	std::vector<std::thread> 			worker_threads;
	concurrent_vector<wait_list_entry>	fiber_wait_list(fiber_count);
	fiber::fiber_pool 					fiber_pool(fiber_count, fiber_func);
	concurrent_queue<task> 				queue_high(queue_high_size);
	concurrent_queue<task> 				queue(queue_size);
	task_system_report 					report;
	std::atomic_bool 					exec_flag;
	std::atomic_size_t					default_wait_counter;
};

std::unique_ptr<task_system> gp_task_system;


class fiber_wait_list {
public:

	fiber_wait_list(size_t);


	void push_back(void* p_fiber, exec_object* p_exec_obj)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		wait_list_.emplace_back(p_fiber, p_exec_obj);
	}

	bool pop(void* p_fiber)
	{
		std::lock_guard<std::mutex> lock;

		if (wait_list_.empty()) return false;

		for (size_t i = wait_list_.size(); i > 0; --i) {
			auto& entry = wait_list_[i - 1];

			if (*entry.p_wait_counter == 0) {
				// remove i
				// place the last to i
				p_fiber = entry.p_fiber;
				return true;
			}
		}

		return false;		
	}


private:

	struct wait_list_entry final {
		void* p_fiber;
		std::atomic_size_t* p_wait_counter;
	};

	mutex_;
	wait_list_;

	fiber_list_;
	counter_list_;
};


void thread_worker_func()
{
	// Gives the specified thread fiber nature.
	fiber::fiber_thread ft(std::this_thread::native_handle());	

	void* p_fbr = [!!!]gp_task_runtime->fiber_pool.pop();
	fiber::switch_to_fiber(p_fbr);
}

void fiber_worker_func(void* data)
{
	while ([!!!]gp_task_system->exec_flag) {
		
		// check wait list
		void* p_fbr;
		bool res = [!!!]gp_task_system->fiber_wait_list.try_pop(p_fbr);
		if (res) {
			[!!!]gp_task_system->fiber_pool.push_back(fiber::current_fiber());		// avoid duplicates
			fiber::switch_to_fiber(fbp_fbrr);	
		}

		// drain priority queue

		// process regular tasks
		task t;
		bool res = [!!!]gp_task_system->queue.pop(t);
		if (res) {
			t.func();
			decrease_wait_counter(t.exec_object);
		}
	}
}

void wait_for(std::atomic_size_t& wait_counter)
{
	assert(wait_counter > 0);
	[!!!]gp_task_runtime->fiber_wait_list.push_back(fiber::current_fiber(), &wait_counter);
	
	void* p_fbr = [!!!]gp_task_runtime->fiber_pool.pop();
	fiber::switch_to_fiber(p_fbr);
}

```