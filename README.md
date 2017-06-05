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


struct task final {
	std::function<void()> func;
	std::atomic_size_t* p_wait_counter;
};

inline void exec_task(task& task)
{
	task.func();
	
	if (task.p_wait_counter) {
		assert(*task.p_wait_counter > 0);
		--(*task.p_wait_counter);
	}
}

class task_system final {
public:

	void run(task_desc* p_tasks, size_t count)
	{
		assert(p_tasks);
		assert(count > 0);

		report_.task_count += count;

		for (size_t i = 0; i < count; ++i) {
			auto& td = p_tasks[i];
			queue_.push(task{ std::move(td.func), nullptr });
		}
	}

	void wait_for(std::atomic_size_t& wait_counter)
	{
		assert(wait_counter > 0);

		// undefined behaviour: fiber is still running, but it's already in the wait list.
		fiber_wait_list_.push_back(fiber::current_fiber(), &wait_counter);
		fiber_pool_.switch_to_next_fiber();
	}

private:

	static void thread_worker_func(fiber_pool& fiber_pool)
	{
		// Gives the specified thread fiber nature.
		fiber::fiber_thread ft(std::this_thread::native_handle());	
		fiber_system.execute_fiber();
	}


	std::vector<std::thread>		worker_threads_;
	fiber::fiber_wait_list			fiber_wait_list_(fiber_count);
	fiber::fiber_pool 				fiber_pool_(fiber_count, fiber_func);
	concurrent_queue<task> 			queue_high_(queue_high_size);
	concurrent_queue<task> 			queue_(queue_size);
	task_system_report 				report_;
	std::atomic_bool 				exec_flag_;
};

struct fiber_worker_context final {
	fiber_system&				fiber_system;				
	concurrent_queue<task>& 	queue_high;
	concurrent_queue<task>& 	queue;
	std::atomic_bool& 			exec_flag;
};

void fiber_worker_func(void* data)
{
	fiber_worker_context ctx = *static_cast<fiber_worker_context*>(data);

	while (ctx.exec_flag) {
		
		ctx.fiber_system.execute_waiting_fibers();

		// drain priority queue

		// process regular tasks
		task t;
		bool res = ctx.queue.pop(t);
		if (res) exec_task(t);
	}
}


class fiber_system final {
public:

	fiber_system(size_t fiber_count, void (*fiber_func)(void*));


	void execute_fiber()
	{
		void* p_fbr = nullptr;

		{
			std::lock_guard<std::mutex> lock(mutex_fiber_pool_);
			p_fbr = fiber_pool_.pop();
		}

		fiber::switch_to_fiber(p_fbr);
	}

	void execute_waiting_fibers()
	{
		// priority tasks


		void* p_fbr;
		bool res = fiber_wait_list.try_pop(p_fbr);
		
		if (res) {
			{
				std::lock_guard<std::mutex> lock(mutex_fiber_pool_);
				// avoid duplicates
				// undefined behaviour: fiber is still running, but it's already in the pool.
				fiber_pool.push_back(fiber::current_fiber());		
			}

			fiber::switch_to_fiber(p_fbr);	
		}
	}

private:

};

```