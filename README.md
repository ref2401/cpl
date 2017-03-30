# tpl

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


## Bibliography
- https://en.wikipedia.org/wiki/Fiber_(computer_science)
- Portable Coroutine Library http://freecode.com/projects/libpcl/
- CFiber http://www.flounder.com/fibers.htm
- http://www.gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine
