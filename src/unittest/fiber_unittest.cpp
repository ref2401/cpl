#include "ts/fiber.h"


#include "CppUnitTest.h"

using ts::fiber_pool;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace {

void fiber_func(void*) { /* noop */ }

} // namespace


namespace unittest {

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

} // namespace unittest
