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
			// Highest pin count T a slot word reached (low 16 bits after a slot fetch_add).
			static inline std::atomic_uint32_t maxPins { 0 };

			static void reset() {
				loads = 0;
				stores = 0;
				rmws = 0;
				maxPins = 0;
			}

			static void observePins(const std::uint32_t pins_) {
				auto seen = maxPins.load();
				while (pins_ > seen && not maxPins.compare_exchange_weak(seen, pins_)) {}
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
				const auto prev = value.fetch_add(arg_, order_);
				// The slot word is the only uintptr_t atomic; the object counter is s64.
				if constexpr (std::same_as<Ux_, std::uintptr_t>) {
					OpCounts::observePins(static_cast<std::uint32_t>((prev + arg_) & 0xFFFFu));
				}
				return prev;
			}

			Ux_ exchange(Ux_ value_, std::memory_order order_ = std::memory_order::seq_cst) noexcept {
				++OpCounts::rmws;
				return value.exchange(value_, order_);
			}

			bool compare_exchange_strong(
				Ux_& expected_,
				Ux_ desired_,
				std::memory_order success_,
				std::memory_order failure_
			) noexcept {
				++OpCounts::rmws;
				return value.compare_exchange_strong(expected_, desired_, success_, failure_);
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
	static_assert(std::same_as<ArcFromThis<TrackedArci>::counter_type, std::atomic<s64>>);

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

	TEST_F(ArciAtomicPolicy, RmwCountsPerOperation) {
		auto a = CountingArci::create();

		// Copy: pin on the source word + live on the object counter.
		OpCounts::reset();
		auto b = a;
		EXPECT_EQ(OpCounts::rmws.load(), 2u);
		EXPECT_EQ(OpCounts::stores.load(), 0u);

		// Move-construct: one exchange on the (possibly shared) source.
		OpCounts::reset();
		auto c = std::move(b);
		EXPECT_EQ(OpCounts::rmws.load(), 1u);

		// Destroy of a non-last word: one fold on the object counter.
		OpCounts::reset();
		{
			auto dropped = std::move(c);
			OpCounts::reset();
		}
		EXPECT_EQ(OpCounts::rmws.load(), 1u);

		// Move-assign into a non-null target: source exchange, target exchange, fold of the old target word.
		auto d = CountingArci::create();
		auto e = a;
		OpCounts::reset();
		d = std::move(e);
		EXPECT_EQ(OpCounts::rmws.load(), 3u);
		EXPECT_EQ(liveCount, 1);

		// Reset: exchange + fold.
		OpCounts::reset();
		d.reset();
		EXPECT_EQ(OpCounts::rmws.load(), 2u);

		// Copy-out from a held reference: one RMW on the object counter.
		OpCounts::reset();
		auto f = a->arci_from_this();
		EXPECT_EQ(OpCounts::rmws.load(), 1u);
		EXPECT_EQ(a.ref_count(), 2u);

		// Null: copy and destroy touch no counter; move-construct still exchanges the source.
		const CountingArci empty {};
		OpCounts::reset();
		{
			auto copy = empty;
			auto moved = std::move(copy);
		}
		EXPECT_EQ(OpCounts::rmws.load(), 1u);

		f.reset();
		a.reset();
		EXPECT_EQ(liveCount, 0);
	}

	TEST_F(ArciAtomicPolicy, PinFlushBoundsSlotPins) {
		constexpr auto flushAt = CountingArci::pin_flush_at;
		auto a = CountingArci::create();

		OpCounts::reset();
		for (int i = 0; i < 3 * flushAt; ++i) {
			auto copy = a;
		}
		// The pin that observes T == flushAt flushes the word; single-threaded T never exceeds flushAt + 1.
		EXPECT_EQ(OpCounts::maxPins.load(), flushAt + 1u);
		EXPECT_EQ(a.ref_count(), 1u);
		EXPECT_EQ(liveCount, 1);

		a.reset();
		EXPECT_EQ(liveCount, 0);
	}

	TEST_F(ArciAtomicPolicy, ConcurrentPinsStayBelowFlushBound) {
		constexpr auto flushAt = CountingArci::pin_flush_at;
		constexpr int threads = 8;
		auto a = CountingArci::create();

		OpCounts::reset();
		std::vector<std::thread> pool {};
		for (int t = 0; t < threads; ++t) {
			pool.emplace_back(
				[&a] {
					for (int i = 0; i < 2 * flushAt; ++i) {
						auto copy = a;
					}
				}
			);
		}
		for (auto& th : pool) { th.join(); }

		EXPECT_GT(OpCounts::maxPins.load(), flushAt);
		EXPECT_LE(OpCounts::maxPins.load(), flushAt + threads);
		EXPECT_EQ(a.ref_count(), 1u);
		EXPECT_EQ(liveCount, 1);

		a.reset();
		EXPECT_EQ(liveCount, 0);
	}
}
