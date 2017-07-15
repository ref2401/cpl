#include "ts/fiber.h"

#include "CppUnitTest.h"

using ts::fiber;
using ts::fiber_pool;
using ts::fiber_wait_list;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace {

void fiber_func(void*) { /* noop */ }

} // namespace


namespace unittest {

TEST_CLASS(fiber_fiber) {
public:

	TEST_METHOD(ctors)
	{
		fiber f0;
		Assert::IsNull(f0.p_handle);

		fiber f1(fiber_func, 128);
		Assert::IsNotNull(f1.p_handle);

		// move ctor
		const void* p_expected_handle = f1.p_handle;
		fiber f_m = std::move(f1);
		Assert::IsNull(f1.p_handle);
		Assert::AreEqual<const void*>(p_expected_handle, f_m.p_handle);
	}

	TEST_METHOD(dispose)
	{
		fiber f;
		Assert::IsNull(f.p_handle);
		f.dispose();
		Assert::IsNull(f.p_handle);

		f = fiber(fiber_func, 128);
		Assert::IsNotNull(f.p_handle);
		f.dispose();
		Assert::IsNull(f.p_handle);
	}
};

TEST_CLASS(fiber_fiber_pool) {
public:

	TEST_METHOD(push_back_pop)
	{
		fiber_pool fiber_pool(3, fiber_func, 128);

		// #0
		void* f0 = fiber_pool.pop();
		Assert::IsNotNull(f0);
		// #1
		void* f1 = fiber_pool.pop();
		Assert::IsNotNull(f1);
		// #2
		void* f2 = fiber_pool.pop();
		Assert::IsNotNull(f2);
		// nullptr
		void* f3 = fiber_pool.pop();
		Assert::IsNull(f3);

		Assert::IsTrue((f0 != f1) && (f1 != f2));

		// push one back
		fiber_pool.push_back(f0);
		void* f4 = fiber_pool.pop();
		Assert::AreEqual(f0, f4);
		// nullptr
		void* f5 = fiber_pool.pop();
		Assert::IsNull(f5);
	}
};

TEST_CLASS(task_system_fiber_wait_list) {
public:

	TEST_METHOD(push_try_pop)
	{
		std::atomic_size_t wc1 = 1;
		std::atomic_size_t wc2 = 2;
		std::atomic_size_t wc3 = 3;
		fiber_wait_list wait_list(3);

		// Try pop from empty wait list
		void* p_fiber = nullptr;
		Assert::IsFalse(wait_list.try_pop(p_fiber));
		Assert::IsNull(p_fiber);

		// Populate the wait list. The test does not use real fibers.
		wait_list.push(&wc1, &wc1);
		wait_list.push(&wc2, &wc2);
		wait_list.push(&wc3, &wc3);

		// All the wait counters are greater than zero.
		p_fiber = nullptr;
		Assert::IsFalse(wait_list.try_pop(p_fiber));
		Assert::IsNull(p_fiber);

		// get back wc1
		--wc1;
		Assert::AreEqual<size_t>(0, wc1);
		Assert::IsTrue(wait_list.try_pop(p_fiber));
		Assert::AreEqual<void*>(p_fiber, &wc1);

		// get back wc2
		p_fiber = nullptr;
		--wc2;
		Assert::AreEqual<size_t>(1, wc2);
		Assert::IsFalse(wait_list.try_pop(p_fiber));
		Assert::IsNull(p_fiber);
		--wc2;
		Assert::AreEqual<size_t>(0, wc2);
		Assert::IsTrue(wait_list.try_pop(p_fiber));
		Assert::AreEqual<void*>(p_fiber, &wc2);

		// get back wc3;
		p_fiber = nullptr;
		--wc3;
		Assert::AreEqual<size_t>(2, wc3);
		Assert::IsFalse(wait_list.try_pop(p_fiber));
		Assert::IsNull(p_fiber);
		wc3 = 0;
		Assert::IsTrue(wait_list.try_pop(p_fiber));
		Assert::AreEqual<void*>(p_fiber, &wc3);

		// The list is empty again
		p_fiber = nullptr;
		Assert::IsFalse(wait_list.try_pop(p_fiber));
		Assert::IsNull(p_fiber);
	}
};

} // namespace unittest
