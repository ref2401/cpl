#include "ts/utility.h"

#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace unittest {

TEST_CLASS(utility_ring_buffer) {
public:

	TEST_METHOD(assign_operator)
	{
		ts::ring_buffer<int> rb(3);
		rb.try_push(1);
		rb.try_push(2);

		// copy assign
		ts::ring_buffer<int> rb_c;
		rb_c = rb;
		Assert::IsFalse(rb_c.empty());
		Assert::AreEqual<size_t>(rb.size(), rb_c.size());
		Assert::AreEqual<size_t>(rb.size_limit(), rb_c.size_limit());

		// move assign
		ts::ring_buffer<int> rb_m;
		rb_m = std::move(rb_c);
		Assert::IsFalse(rb_m.empty());
		Assert::AreEqual<size_t>(rb.size(), rb_m.size());
		Assert::AreEqual<size_t>(rb.size_limit(), rb_m.size_limit());
		Assert::IsTrue(rb_c.empty());
		Assert::AreEqual<size_t>(0, rb_c.size());
		Assert::AreEqual<size_t>(0, rb_c.size_limit());
	}

	TEST_METHOD(ctor)
	{
		ts::ring_buffer<int> rb_0;
		Assert::IsTrue(rb_0.empty());
		Assert::AreEqual<size_t>(0, rb_0.size());
		Assert::AreEqual<size_t>(0, rb_0.size_limit());

		ts::ring_buffer<int> rb_1(3);
		Assert::IsTrue(rb_1.empty());
		Assert::AreEqual<size_t>(0, rb_1.size());
		Assert::AreEqual<size_t>(3, rb_1.size_limit());

		// copy ctor
		rb_1.try_push(1);
		rb_1.try_push(2);
		ts::ring_buffer<int> rb_c = rb_1;
		Assert::IsFalse(rb_c.empty());
		Assert::AreEqual<size_t>(rb_1.size(), rb_c.size());
		Assert::AreEqual<size_t>(rb_1.size_limit(), rb_c.size_limit());

		// move ctor
		ts::ring_buffer<int> rb_m = std::move(rb_c);
		Assert::IsFalse(rb_m.empty());
		Assert::AreEqual<size_t>(rb_1.size(), rb_m.size());
		Assert::AreEqual<size_t>(rb_1.size_limit(), rb_m.size_limit());
		Assert::IsTrue(rb_c.empty());
		Assert::AreEqual<size_t>(0, rb_c.size());
		Assert::AreEqual<size_t>(0, rb_c.size_limit());
	}

	TEST_METHOD(try_emplace)
	{
		struct payload {
			int value;
		};

		ts::ring_buffer<payload> queue(2);
		// push 1
		Assert::IsTrue(queue.try_emplace(1));
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(1, queue.size());
		// push 2
		Assert::IsTrue(queue.try_emplace(2));
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(2, queue.size());
		// emplace fails due to size_limit
		Assert::IsFalse(queue.try_emplace(42));
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(2, queue.size());

		payload pl;
		// pop 1
		Assert::IsTrue(queue.try_pop(pl));
		Assert::AreEqual(1, pl.value);
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(1, queue.size());
		// pop 2
		Assert::IsTrue(queue.try_pop(pl));
		Assert::AreEqual(2, pl.value);
		Assert::IsTrue(queue.empty());
		Assert::AreEqual<size_t>(0, queue.size());
		// pop fails due to size_limit
		Assert::IsFalse(queue.try_pop(pl));
		Assert::IsTrue(queue.empty());
		Assert::AreEqual<size_t>(0, queue.size());
	}
	
	TEST_METHOD(try_push_try_pop)
	{
		ts::ring_buffer<int> queue(3);

		// push 1
		Assert::IsTrue(queue.try_push(1));
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(1, queue.size());
		// push 2
		Assert::IsTrue(queue.try_push(2));
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(2, queue.size());
		// push 3
		Assert::IsTrue(queue.try_push(3));
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(3, queue.size());
		// push fails due to size_limit
		Assert::IsFalse(queue.try_push(42));
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(3, queue.size());

		int v;
		// pop 1
		Assert::IsTrue(queue.try_pop(v));
		Assert::AreEqual(1, v);
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(2, queue.size());
		// pop 2
		Assert::IsTrue(queue.try_pop(v));
		Assert::AreEqual(2, v);
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(1, queue.size());
		// pop 3
		Assert::IsTrue(queue.try_pop(v));
		Assert::AreEqual(3, v);
		Assert::IsTrue(queue.empty());
		Assert::AreEqual<size_t>(0, queue.size());
		// pop fails due to size_limit
		Assert::IsFalse(queue.try_pop(v));
		Assert::IsTrue(queue.empty());
		Assert::AreEqual<size_t>(0, queue.size());

		// test index circling in the buffer_
		// we can't check indices directly but we can make sure the buffer_ works fine
		// push 4
		Assert::IsTrue(queue.try_push(4));
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(1, queue.size());
		// push 5
		Assert::IsTrue(queue.try_push(5));
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(2, queue.size());
		// pop 4
		Assert::IsTrue(queue.try_pop(v));
		Assert::AreEqual(4, v);
		Assert::IsFalse(queue.empty());
		Assert::AreEqual<size_t>(1, queue.size());
		// pop 5
		Assert::IsTrue(queue.try_pop(v));
		Assert::AreEqual(5, v);
		Assert::IsTrue(queue.empty());
		Assert::AreEqual<size_t>(0, queue.size());
	}

	TEST_METHOD(try_push_try_pop_empty_buffer)
	{
			ts::ring_buffer<int> queue;
			Assert::IsFalse(queue.try_push(24));

			Assert::IsTrue(queue.empty());
			Assert::AreEqual<size_t>(0, queue.size());

			int origin_value = 1000;
			int v = origin_value;
			Assert::IsFalse(queue.try_pop(v));
			Assert::AreEqual(origin_value, v); // v has not been changed
	}
};

} // namespace unittest
