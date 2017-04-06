#include "cpl/concurrent_queue.h"

#include <vector>
#include "CppUnitTest.h"

using cpl::concurrent_queue;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace unittest {	

TEST_CLASS(concurrent_queue_concurrent_queue) {
public:

	TEST_METHOD(ctors)
	{
		concurrent_queue<int> queue;
		Assert::IsTrue(queue.empty());
		Assert::AreEqual<size_t>(0, queue.size());
	}

	TEST_METHOD(push_pop_one_thread)
	{
		std::unique_ptr<int> p0 = std::make_unique<int>(24);
		std::unique_ptr<int> p1 = std::make_unique<int>(100);

		// push
		concurrent_queue<std::unique_ptr<int>> queue;
		queue.push(std::move(p0));
		queue.push(std::move(p1));

		Assert::IsFalse(bool(p0));
		Assert::IsFalse(bool(p1));
		Assert::AreEqual<size_t>(2, queue.size());

		// pop
		std::unique_ptr<int> out;

		// pop 24 back
		Assert::IsTrue(queue.try_pop(out));
		Assert::AreEqual(24, *out);
		Assert::AreEqual<size_t>(1, queue.size());

		// pop 100 back
		Assert::IsTrue(queue.wait_pop(out));
		Assert::AreEqual(100, *out);
		Assert::IsTrue(queue.empty());
		Assert::AreEqual<size_t>(0, queue.size());
	}
};

} // namespace
