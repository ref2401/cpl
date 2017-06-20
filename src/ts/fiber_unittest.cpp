#include "ts/fiber.h"

#include "CppUnitTest.h"

using ts::fiber;
using ts::fiber_pool;
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

} // namespace unittest
