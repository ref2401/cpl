# cpl

## steps
- fiber
- 2 fibers cout << "something". test
- wait_for
- concurrent_queue, ABA problem.

Examples:
- map reduce
- scan
- fork-join (task_counter)
- some examples from structured parallel programming
 

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
// thread.h

struct fiber_nature_thread;
class fiber_pool;

void* current_fiber();
void* fiber_data();
switch_to_fiber(void* p_fiber);


// task_system.h

class fiber_wait_list {
public:

	fiber_wait_list(size_t);


	void push_back(void* p_fiber, std::atomic_size_t* p_wait_counter)
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
				p_fiber = entry.p_fiber;
				wait_list_.remove_at(i);
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

	concurrent_vector<wait_list_entry> wait_list_;
};


struct task_system_state final {

	task_system_state(size_t queue_high_size, size_t queue_size);

	concurrent_queue<task> 	queue_high;
	concurrent_queue<task> 	queue;
	std::atomic_bool		exec_flag;
};

struct worker_fiber_context final {
	static thread_local void* 				p_thread_main_fiber = nullptr;
	static thread_local std::atomic_size_t* p_wait_list_counter_ = nullptr;
};

class task_system final {
public:

	task_system(...);


	void run(task_desc* p_tasks, size_t count)
	{
		assert(p_tasks);
		assert(count > 0);

		report_.task_count += count;

		for (size_t i = 0; i < count; ++i) {
			auto& td = p_tasks[i];
			state_.queue.push(task{ std::move(td.func), nullptr });
		}
	}

	void wait_for(std::atomic_size_t& wait_counter)
	{
		assert(wait_counter > 0);

		worker_fiber_context::p_wait_list_counter_ = &wait_counter;
		switch_to_fiber(worker_fiber_context::p_thread_main_fiber);
	}

private:

	static void worker_thread_main_func(fiber_pool& fiber_pool, fiber_wait_list& fiber_wait_list, std::atomic_bool& exec_flag)
	{
		// init the worker thread
		thread_main_fiber 	tmf;	
		void* 				p_fiber_to_exec = fiber_pool_.pop();
		
		// init fiber execution context
		worker_fiber_context::p_thread_main_fiber = tmf.p_fiber;
		worker_fiber_context::p_wait_list_counter = nullptr;
		
		// main loop
		while (exec_flag) {
			switch_to_fiber(p_fiber_to_exec);

			if (worker_fiber_context::p_wait_list_counter) {
				fiber_wait_list.push_back(p_fiber_to_exec, worker_fiber_context::p_wait_list_counter);
				worker_fiber_context::p_wait_list_counter = nullptr;

				p_fiber_to_exec = fiber_pool.pop();
			}
			else {
				// waiting fiber
				void* p_fpr;
				bool res = fiber_wait_list.pop(p_fpr);

				if (res) {
					fiber_pool.push_back(p_fiber_to_exec);
					p_fiber_to_exec = p_fpr;
				}
			}
		} // while

	}

	std::vector<std::thread>	worker_threads_;
	fiber::fiber_wait_list		fiber_wait_list_(fiber_count);
	fiber::fiber_pool 			fiber_pool_(fiber_count, fiber_func);
	task_system_state			state_;
	task_system_report 			report_;
};


inline void exec_task(task& task)
{
	task.func();
	
	if (task.p_wait_counter) {
		assert(*task.p_wait_counter > 0);
		--(*task.p_wait_counter);
	}
}

void worker_fiber_func(void* data)
{
	task_system_state& state = *static_cast<task_system_state*>(data);

	while (state.exec_flag) {
		// drain priority queue

		// process regular tasks
		task t;
		bool res = state.queue.try_pop(t);
		if (res) exec_task(t);

		switch_to_fiber(worker_fiber_context::p_thread_main_fiber);
	}
}

```