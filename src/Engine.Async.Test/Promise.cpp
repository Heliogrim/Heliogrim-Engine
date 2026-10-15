#include "pch.h"

#include <string>
#include <thread>
#include <Engine.Async/Await/Promise.hpp>

using namespace hg::concurrent;

namespace Async {
	TEST(Promise, FutureReceivesResult) {
		promise<int> p { []() -> int { return 5; } };
		auto fut = p.get();

		EXPECT_FALSE(fut.ready());
		p();

		ASSERT_TRUE(fut.ready());
		EXPECT_EQ(fut.get(), 5);
	}

	TEST(Promise, ThenChainsResults) {
		promise<int> p { []() -> int { return 2; } };
		auto next = p.then<std::string>([](int v_) { return std::to_string(v_ * 21); });
		auto fut = next.get();

		p();

		ASSERT_TRUE(fut.ready());
		EXPECT_EQ(fut.get(), "42");
	}

	TEST(Promise, FinallyReceivesResult) {
		promise<int> p { []() -> int { return 7; } };

		int seen = 0;
		p.finally([&seen](int v_) { seen = v_; });
		p();

		EXPECT_EQ(seen, 7);
	}

	TEST(Promise, FutureAfterThenThrows) {
		promise<int> p { []() -> int { return 1; } };
		auto next = p.then<int>([](int v_) { return v_; });

		EXPECT_THROW((void)p.get(), std::logic_error);
	}

	TEST(Promise, VoidPromiseCompletesFuture) {
		bool ran = false;
		promise<void> p { [&ran]() { ran = true; } };
		auto fut = p.get();

		EXPECT_FALSE(fut.ready());
		p();

		EXPECT_TRUE(ran);
		EXPECT_TRUE(fut.ready());
	}

	TEST(Promise, FutureGetWaitsForOtherThread) {
		promise<int> p { []() -> int { return 99; } };
		auto fut = p.get();

		std::thread worker([task = std::move(p)]() mutable { task(); });

		EXPECT_EQ(fut.get(), 99);
		worker.join();
	}

	TEST(Promise, InvokingTwiceCompletesFutureOnlyOnce) {
		promise<int> p { []() -> int { return 1; } };
		auto fut = p.get();

		p();
		// Function is unlinked after the first step; a second step must not silently succeed.
		EXPECT_ANY_THROW(p());
		EXPECT_EQ(fut.get(), 1);
	}
}
