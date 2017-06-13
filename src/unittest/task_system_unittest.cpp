#include "ts/task_system.h"

#include "CppUnitTest.h"

using ts::fiber_wait_list;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace unittest {

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
