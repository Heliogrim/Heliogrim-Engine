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

	/**/

	namespace {
		struct OpCounts {
			static inline std::atomic_size_t loads { 0 };
			static inline std::atomic_size_t stores { 0 };
			static inline std::atomic_size_t rmws { 0 };

			static void reset() {
				loads = 0;
				stores = 0;
				rmws = 0;
			}
		};

		template <class Ux_>
		struct CountingAtomic {
			std::atomic<Ux_> value;

			constexpr CountingAtomic(Ux_ value_) noexcept :
				value(value_) {}

			Ux_ load(std::memory_order order_ = std::memory_order::seq_cst) const noexcept {
				++OpCounts::loads;
				return value.load(order_);
			}

			void store(Ux_ value_, std::memory_order order_ = std::memory_order::seq_cst) noexcept {
				++OpCounts::stores;
				value.store(value_, order_);
			}

			Ux_ fetch_add(Ux_ arg_, std::memory_order order_ = std::memory_order::seq_cst) noexcept {
				++OpCounts::rmws;
				return value.fetch_add(arg_, order_);
			}

			Ux_ fetch_sub(Ux_ arg_, std::memory_order order_ = std::memory_order::seq_cst) noexcept {
				++OpCounts::rmws;
				return value.fetch_sub(arg_, order_);
			}

			Ux_ fetch_and(Ux_ arg_, std::memory_order order_ = std::memory_order::seq_cst) noexcept {
				++OpCounts::rmws;
				return value.fetch_and(arg_, order_);
			}

			Ux_ fetch_or(Ux_ arg_, std::memory_order order_ = std::memory_order::seq_cst) noexcept {
				++OpCounts::rmws;
				return value.fetch_or(arg_, order_);
			}

			Ux_ operator++() noexcept {
				++OpCounts::rmws;
				return ++value;
			}

			Ux_ operator--() noexcept {
				++OpCounts::rmws;
				return --value;
			}

			Ux_ operator-=(Ux_ arg_) noexcept {
				++OpCounts::rmws;
				return value -= arg_;
			}
		};

		struct CountingPolicy {
			template <class Ux_>
			using atomic = CountingAtomic<Ux_>;

			static void fence(std::memory_order order_) noexcept {
				std::atomic_thread_fence(order_);
			}
		};

		struct CountedArci :
			public ArcFromThis<CountedArci, CountingPolicy> {
			CountedArci() { ++liveCount; }

			~CountedArci() { --liveCount; }
		};

		using CountingArci = Arci<CountedArci, CountingPolicy>;
	}

	static_assert(sizeof(Arci<TrackedArci>) == sizeof(std::atomic_uintptr_t));
	static_assert(std::same_as<ArcFromThis<TrackedArci>::counter_type, std::atomic_uint_fast16_t>);

	using ArciAtomicPolicy = LifetimeFixture;

	TEST_F(ArciAtomicPolicy, SemanticsHoldThroughCountingPolicy) {
		{
			auto a = CountingArci::create();
			EXPECT_EQ(liveCount, 1);
			EXPECT_EQ(a.ref_count(), 1);

			auto b = a;
			EXPECT_EQ(a.ref_count(), 2);

			auto c = std::move(b);
			EXPECT_TRUE(b == nullptr);
			EXPECT_EQ(a.ref_count(), 2);

			auto d = CountingArci::create();
			EXPECT_EQ(liveCount, 2);
			d = std::move(c);
			EXPECT_EQ(liveCount, 1);
			EXPECT_EQ(a.ref_count(), 2);

			Arci<const CountedArci, CountingPolicy> e = a;
			EXPECT_EQ(a.ref_count(), 3);
			EXPECT_EQ(e.get(), a.get());

			auto f = a->arci_from_this();
			EXPECT_EQ(a.ref_count(), 4);

			f.reset();
			e.reset();
			d.reset();
			EXPECT_EQ(a.ref_count(), 1);
			EXPECT_EQ(liveCount, 1);
		}
		EXPECT_EQ(liveCount, 0);
	}

	TEST_F(ArciAtomicPolicy, AtomicOpsAreObservable) {
		auto a = CountingArci::create();

		OpCounts::reset();
		auto b = a;
		EXPECT_GE(OpCounts::rmws.load(), 1u);

		OpCounts::reset();
		(void)a.ref_count();
		EXPECT_GE(OpCounts::loads.load(), 1u);

		OpCounts::reset();
		b.reset();
		a.reset();
		EXPECT_GE(OpCounts::rmws.load(), 2u);
		EXPECT_EQ(liveCount, 0);
	}
}
