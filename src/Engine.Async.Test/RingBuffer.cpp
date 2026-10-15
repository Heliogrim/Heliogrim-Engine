#include "pch.h"

#include <atomic>
#include <thread>
#include <Engine.Async/Collection/RingBuffer.hpp>

using namespace hg;
using namespace hg::engine::concurrent;

namespace Async {
	TEST(AsyncRingBuffer, StartsEmpty) {
		RingBuffer<int> rb { 8uL };
		int out = 0;

		EXPECT_TRUE(rb.empty());
		EXPECT_FALSE(rb.full());
		EXPECT_FALSE(rb.try_pop(out));
	}

	TEST(AsyncRingBuffer, PushPopIsFifo) {
		RingBuffer<int> rb { 8uL };

		EXPECT_TRUE(rb.try_push(1));
		EXPECT_TRUE(rb.try_push(2));
		EXPECT_TRUE(rb.try_push(3));
		EXPECT_FALSE(rb.empty());

		int out = 0;
		for (int expected = 1; expected <= 3; ++expected) {
			ASSERT_TRUE(rb.try_pop(out));
			EXPECT_EQ(out, expected);
		}
		EXPECT_FALSE(rb.try_pop(out));
		EXPECT_TRUE(rb.empty());
	}

	TEST(AsyncRingBuffer, RejectsPushWhenFull) {
		RingBuffer<int> rb { 4uL };

		size_t accepted = 0uLL;
		for (int i = 0; i < 16; ++i) {
			accepted += rb.try_push(int { i }) ? 1uLL : 0uLL;
		}

		// One slot stays free to tell full from empty.
		EXPECT_EQ(accepted, 3uLL);
		EXPECT_TRUE(rb.full());
	}

	TEST(AsyncRingBuffer, WrapsAround) {
		RingBuffer<int> rb { 4uL };

		int out = 0;
		for (int i = 0; i < 40; ++i) {
			ASSERT_TRUE(rb.try_push(int { i })) << "push " << i;
			ASSERT_TRUE(rb.try_pop(out)) << "pop " << i;
			EXPECT_EQ(out, i);
		}
		EXPECT_TRUE(rb.empty());
	}

	TEST(AsyncRingBuffer, FillDrainRefill) {
		RingBuffer<int> rb { 4uL };
		int out = 0;

		for (int round = 0; round < 5; ++round) {
			for (int i = 1; i <= 3; ++i) { EXPECT_TRUE(rb.try_push(10 * round + i)); }
			EXPECT_FALSE(rb.try_push(99));

			for (int i = 1; i <= 3; ++i) {
				ASSERT_TRUE(rb.try_pop(out));
				EXPECT_EQ(out, 10 * round + i);
			}
			EXPECT_FALSE(rb.try_pop(out));
		}
	}

	TEST(AsyncRingBuffer, SingleProducerSingleConsumerKeepsOrder) {
		RingBuffer<int> rb { 64uL };
		constexpr int total = 20000;

		std::thread producer(
			[&rb] {
				for (int i = 0; i < total; ++i) {
					while (not rb.try_push(int { i })) { std::this_thread::yield(); }
				}
			}
		);

		int next = 0;
		int out = 0;
		while (next < total) {
			if (rb.try_pop(out)) {
				ASSERT_EQ(out, next);
				++next;
			} else {
				std::this_thread::yield();
			}
		}
		producer.join();

		EXPECT_TRUE(rb.empty());
	}
}
