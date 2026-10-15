#include "pch.h"

#include <atomic>
#include <thread>
#include <vector>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Managed/AtomicRefCountedIntrusive.hpp>
#include <Engine.Common/Managed/RefCountedIntrusive.hpp>

using namespace hg;

namespace Common {
	namespace {
		inline std::atomic_int liveCount { 0 };

		struct Tracked {
			Tracked() { ++liveCount; }

			~Tracked() { --liveCount; }
		};

		struct TrackedRci :
			public RcFromThis<TrackedRci> {
			TrackedRci() { ++liveCount; }

			~TrackedRci() { --liveCount; }
		};

		struct TrackedArci :
			public ArcFromThis<TrackedArci> {
			TrackedArci() { ++liveCount; }

			~TrackedArci() { --liveCount; }
		};

		struct LifetimeFixture :
			public ::testing::Test {
			void SetUp() override { liveCount = 0; }
		};
	}

	using RcLifetime = LifetimeFixture;

	TEST_F(RcLifetime, RcDestroysOnLastReference) {
		{
			auto a = Rc<Tracked>::create();
			EXPECT_EQ(liveCount, 1);
			{
				auto b = a;
				EXPECT_EQ(a.ref_count(), 2);
			}
			EXPECT_EQ(a.ref_count(), 1);
			EXPECT_EQ(liveCount, 1);
		}
		EXPECT_EQ(liveCount, 0);
	}

	TEST_F(RcLifetime, RcResetDestroys) {
		auto a = Rc<Tracked>::create();
		a.reset();

		EXPECT_EQ(liveCount, 0);
		EXPECT_EQ(a.ref_count(), 0);
		EXPECT_TRUE(a == nullptr);
	}

	TEST_F(RcLifetime, RcNullAccessThrows) {
		Rc<Tracked> empty {};
		EXPECT_THROW((void)empty.operator->(), std::runtime_error);
	}

	TEST_F(RcLifetime, RciDestroysOnLastReference) {
		{
			auto a = Rci<TrackedRci>::create();
			EXPECT_EQ(liveCount, 1);
			{
				auto b = a;
				EXPECT_EQ(a.ref_count(), 2);
			}
			EXPECT_EQ(liveCount, 1);
		}
		EXPECT_EQ(liveCount, 0);
	}

	TEST_F(RcLifetime, RciFromThisSharesCount) {
		auto a = Rci<TrackedRci>::create();
		auto b = a->rci_from_this();

		EXPECT_EQ(a.ref_count(), 2);
		b.reset();
		EXPECT_EQ(a.ref_count(), 1);
	}

	TEST_F(RcLifetime, ArciDestroysOnLastReference) {
		{
			auto a = Arci<TrackedArci>::create();
			{
				auto b = a;
				EXPECT_EQ(a.ref_count(), 2);
			}
			EXPECT_EQ(liveCount, 1);
		}
		EXPECT_EQ(liveCount, 0);
	}

	TEST_F(RcLifetime, ArciConcurrentCopiesBalance) {
		auto shared = Arci<TrackedArci>::create();

		constexpr int threads = 8;
		constexpr int iterations = 2000;

		std::vector<std::thread> pool {};
		for (int t = 0; t < threads; ++t) {
			pool.emplace_back(
				[&shared] {
					for (int i = 0; i < iterations; ++i) {
						auto local = shared;
						auto moved = std::move(local);
					}
				}
			);
		}
		for (auto& th : pool) { th.join(); }

		EXPECT_EQ(shared.ref_count(), 1);
		EXPECT_EQ(liveCount, 1);

		shared.reset();
		EXPECT_EQ(liveCount, 0);
	}
}
