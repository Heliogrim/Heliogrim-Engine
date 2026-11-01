#include "pch.h"

#include <atomic>
#include <thread>
#include <vector>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Managed/Rc.hpp>

using namespace hg;

namespace Common {
	namespace {
		inline std::atomic_int semLive { 0 };

		struct Base {
			int value;

			explicit Base(const int value_ = 0) :
				value(value_) { ++semLive; }

			virtual ~Base() { --semLive; }
		};

		struct Derived : Base {
			using Base::Base;
		};

		struct IntrusiveBase :
			public RcFromThis<IntrusiveBase> {
			int value;

			explicit IntrusiveBase(const int value_ = 0) :
				value(value_) { ++semLive; }

			~IntrusiveBase() { --semLive; }
		};

		struct AtomicIntrusive :
			public ArcFromThis<AtomicIntrusive> {
			int value;

			explicit AtomicIntrusive(const int value_ = 0) :
				value(value_) { ++semLive; }

			~AtomicIntrusive() { --semLive; }
		};

		inline std::atomic_int guardedCreated { 0 };
		inline std::atomic_int guardedDestroyed { 0 };

		// Detects use after free (magic cleared on destroy) and double free (destroy count).
		struct Guarded :
			public ArcFromThis<Guarded> {
			static constexpr int alive = 0x5AFE;

			int magic = alive;
			int value;

			explicit Guarded(const int value_ = 0) :
				value(value_) { ++guardedCreated; }

			~Guarded() {
				magic = 0;
				++guardedDestroyed;
			}

			[[nodiscard]] bool valid() const { return magic == alive; }
		};

		struct SemanticsFixture :
			public ::testing::Test {
			void SetUp() override {
				semLive = 0;
				guardedCreated = 0;
				guardedDestroyed = 0;
			}
		};
	}

	using RcSemantics = SemanticsFixture;
	using RciSemantics = SemanticsFixture;
	using ArciSemantics = SemanticsFixture;
	using ArcSemantics = SemanticsFixture;

	/* Rc */

	TEST_F(RcSemantics, ReadAfterMove) {
		auto src = Rc<Base>::create(11);
		auto dst = std::move(src);

		EXPECT_TRUE(src == nullptr);
		EXPECT_EQ(src.ref_count(), 0);
		EXPECT_THROW((void)src->value, std::runtime_error);

		EXPECT_EQ(dst->value, 11);
		EXPECT_EQ(dst.ref_count(), 1);
		EXPECT_EQ(semLive, 1);
	}

	TEST_F(RcSemantics, MovedFromIsReassignable) {
		auto src = Rc<Base>::create(1);
		auto dst = std::move(src);

		src = Rc<Base>::create(2);
		EXPECT_EQ(src->value, 2);
		EXPECT_EQ(dst->value, 1);
		EXPECT_EQ(semLive, 2);
	}

	TEST_F(RcSemantics, CopyAssignReleasesPreviousTarget) {
		auto a = Rc<Base>::create(1);
		auto b = Rc<Base>::create(2);

		a = b;

		EXPECT_EQ(semLive, 1);
		EXPECT_EQ(a->value, 2);
		EXPECT_EQ(b.ref_count(), 2);
	}

	TEST_F(RcSemantics, MoveAssignReleasesPreviousTarget) {
		auto a = Rc<Base>::create(1);
		auto b = Rc<Base>::create(2);

		a = std::move(b);

		EXPECT_EQ(semLive, 1);
		EXPECT_EQ(a->value, 2);
		EXPECT_EQ(a.ref_count(), 1);
		EXPECT_TRUE(b == nullptr);
	}

	TEST_F(RcSemantics, SelfAssignKeepsObject) {
		auto a = Rc<Base>::create(5);
		auto& alias = a;

		a = alias;
		EXPECT_EQ(a.ref_count(), 1);

		a = std::move(alias);
		EXPECT_EQ(a.ref_count(), 1);
		EXPECT_EQ(a->value, 5);
		EXPECT_EQ(semLive, 1);
	}

	TEST_F(RcSemantics, AssignEmptyReleasesTarget) {
		auto a = Rc<Base>::create(5);
		const Rc<Base> empty {};

		a = empty;

		EXPECT_TRUE(a == nullptr);
		EXPECT_EQ(semLive, 0);
	}

	TEST_F(RcSemantics, CopyOfEmptyStaysEmpty) {
		const Rc<Base> empty {};
		auto copy = empty;

		EXPECT_TRUE(copy == nullptr);
		EXPECT_EQ(copy.ref_count(), 0);
	}

	TEST_F(RcSemantics, CopiesShareObject) {
		auto a = Rc<Base>::create(3);
		auto b = a;

		b->value = 4;
		EXPECT_EQ(a->value, 4);
		EXPECT_EQ(a.operator->(), b.operator->());
	}

	TEST_F(RcSemantics, ResetOneCopyKeepsOthersAlive) {
		auto a = Rc<Base>::create(3);
		auto b = a;

		a.reset();

		EXPECT_EQ(semLive, 1);
		EXPECT_EQ(b->value, 3);
		EXPECT_EQ(b.ref_count(), 1);
	}

	TEST_F(RcSemantics, ReleaseOnlyWhenUnique) {
		auto a = Rc<Base>::create(3);
		auto b = a;

		auto shared = a.release();
		EXPECT_TRUE(shared == nullptr);
		EXPECT_EQ(semLive, 1);
		EXPECT_EQ(b.ref_count(), 2);
	}

	TEST_F(RcSemantics, MoveConstructedFromDerived) {
		auto derived = Rc<Derived>::create(8);
		Rc<Base> base { std::move(derived) };

		EXPECT_TRUE(derived == nullptr);
		EXPECT_EQ(base->value, 8);
		EXPECT_EQ(base.ref_count(), 1);

		base.reset();
		EXPECT_EQ(semLive, 0);
	}

	/* Rci */

	TEST_F(RciSemantics, ReadAfterMove) {
		auto src = Rci<IntrusiveBase>::create(11);
		auto dst = std::move(src);

		EXPECT_TRUE(src.get() == nullptr);
		EXPECT_EQ(src.ref_count(), 0);
		EXPECT_EQ(dst->value, 11);
		EXPECT_EQ(dst.ref_count(), 1);
		EXPECT_EQ(semLive, 1);
	}

	TEST_F(RciSemantics, MoveAssignReleasesPreviousTarget) {
		auto a = Rci<IntrusiveBase>::create(1);
		auto b = Rci<IntrusiveBase>::create(2);

		a = std::move(b);

		EXPECT_EQ(semLive, 1);
		EXPECT_EQ(a->value, 2);
		EXPECT_TRUE(b.get() == nullptr);
	}

	TEST_F(RciSemantics, ResetOneCopyKeepsOthersAlive) {
		auto a = Rci<IntrusiveBase>::create(3);
		auto b = a;

		a.reset();

		EXPECT_EQ(semLive, 1);
		EXPECT_EQ(b->value, 3);
		EXPECT_EQ(b.ref_count(), 1);
	}

	TEST_F(RciSemantics, FromThisRoundTripKeepsObjectAlive) {
		auto a = Rci<IntrusiveBase>::create(3);
		{
			auto b = a->rci_from_this();
			b->value = 6;
		}

		EXPECT_EQ(a->value, 6);
		EXPECT_EQ(a.ref_count(), 1);
		EXPECT_EQ(semLive, 1);
	}

	/* Arci */

	TEST_F(ArciSemantics, ReadAfterMove) {
		auto src = Arci<AtomicIntrusive>::create(11);
		auto dst = std::move(src);

		EXPECT_TRUE(src == nullptr);
		EXPECT_EQ(src.ref_count(), 0);
		EXPECT_EQ(dst->value, 11);
		EXPECT_EQ(dst.ref_count(), 1);
		EXPECT_EQ(semLive, 1);
	}

	TEST_F(ArciSemantics, MovedFromIsReassignable) {
		auto src = Arci<AtomicIntrusive>::create(1);
		auto dst = std::move(src);

		src = Arci<AtomicIntrusive>::create(2);
		EXPECT_EQ(src->value, 2);
		EXPECT_EQ(dst->value, 1);
		EXPECT_EQ(semLive, 2);
	}

	TEST_F(ArciSemantics, MoveAssignReleasesPreviousTarget) {
		auto a = Arci<AtomicIntrusive>::create(1);
		auto b = Arci<AtomicIntrusive>::create(2);

		a = std::move(b);

		EXPECT_EQ(semLive, 1);
		EXPECT_EQ(a->value, 2);
		EXPECT_EQ(a.ref_count(), 1);
		EXPECT_TRUE(b == nullptr);
	}

	TEST_F(ArciSemantics, MoveAssignKeepsOtherCopiesOfPreviousTargetAlive) {
		auto a = Arci<AtomicIntrusive>::create(1);
		auto keep = a;
		auto b = Arci<AtomicIntrusive>::create(2);

		a = std::move(b);

		EXPECT_EQ(semLive, 2);
		EXPECT_EQ(keep->value, 1);
		EXPECT_EQ(keep.ref_count(), 1);
		EXPECT_EQ(a->value, 2);
	}

	TEST_F(ArciSemantics, MoveAssignIntoEmpty) {
		Arci<AtomicIntrusive> a {};
		a = Arci<AtomicIntrusive>::create(4);

		EXPECT_EQ(a->value, 4);
		EXPECT_EQ(a.ref_count(), 1);
		EXPECT_EQ(semLive, 1);
	}

	TEST_F(ArciSemantics, MoveAssignFromEmptyDropsTarget) {
		auto a = Arci<AtomicIntrusive>::create(4);
		a = Arci<AtomicIntrusive> {};

		EXPECT_TRUE(a == nullptr);
		EXPECT_EQ(semLive, 0);
	}

	TEST_F(ArciSemantics, CopyOfEmptyStaysEmpty) {
		const Arci<AtomicIntrusive> empty {};
		auto copy = empty;

		EXPECT_TRUE(copy == nullptr);
		EXPECT_EQ(copy.ref_count(), 0);
	}

	TEST_F(ArciSemantics, ResetOneCopyKeepsOthersAlive) {
		auto a = Arci<AtomicIntrusive>::create(3);
		auto b = a;

		a.reset();

		EXPECT_EQ(semLive, 1);
		EXPECT_EQ(b->value, 3);
		EXPECT_EQ(b.ref_count(), 1);
	}

	TEST_F(ArciSemantics, ConcurrentCopyReadOfSharedSource) {
		auto shared = Arci<AtomicIntrusive>::create(42);

		std::atomic_int mismatches { 0 };
		std::vector<std::thread> pool {};
		for (int t = 0; t < 8; ++t) {
			pool.emplace_back(
				[&] {
					for (int i = 0; i < 2000; ++i) {
						auto local = shared;
						if (local->value != 42) {
							++mismatches;
						}
					}
				}
			);
		}
		for (auto& th : pool) { th.join(); }

		EXPECT_EQ(mismatches, 0);
		EXPECT_EQ(shared.ref_count(), 1);
		EXPECT_EQ(semLive, 1);
	}

	TEST_F(ArciSemantics, ConcurrentMoveAssignToPrivateTargets) {
		std::vector<std::thread> pool {};
		for (int t = 0; t < 8; ++t) {
			pool.emplace_back(
				[t] {
					Arci<AtomicIntrusive> own {};
					for (int i = 0; i < 2000; ++i) {
						own = Arci<AtomicIntrusive>::create(t);
					}
					EXPECT_EQ(own->value, t);
				}
			);
		}
		for (auto& th : pool) { th.join(); }

		EXPECT_EQ(semLive, 0);
	}

	TEST_F(ArciSemantics, ConcurrentWritersToSharedSlot) {
		Arci<AtomicIntrusive> slot { Arci<AtomicIntrusive>::create(0) };

		std::vector<std::thread> pool {};
		for (int t = 1; t <= 4; ++t) {
			pool.emplace_back(
				[&slot, t] {
					for (int i = 0; i < 1000; ++i) {
						slot = Arci<AtomicIntrusive>::create(t);
					}
				}
			);
		}
		for (auto& th : pool) { th.join(); }

		// Exactly one object survives and it is one of the written values.
		EXPECT_EQ(semLive, 1);
		EXPECT_EQ(slot.ref_count(), 1);
		EXPECT_GE(slot->value, 1);
		EXPECT_LE(slot->value, 4);

		slot.reset();
		EXPECT_EQ(semLive, 0);
	}

	TEST_F(ArciSemantics, ConcurrentReadersAndWritersOfSharedSlot) {
		Arci<AtomicIntrusive> slot { Arci<AtomicIntrusive>::create(1) };

		std::atomic_bool stop { false };
		std::atomic_int invalid { 0 };

		std::vector<std::thread> readers {};
		for (int t = 0; t < 4; ++t) {
			readers.emplace_back(
				[&] {
					while (not stop.load()) {
						auto snapshot = slot;
						if (snapshot != nullptr && (snapshot->value < 1 || snapshot->value > 2)) {
							++invalid;
						}
					}
				}
			);
		}

		std::vector<std::thread> writers {};
		for (int t = 1; t <= 2; ++t) {
			writers.emplace_back(
				[&slot, t] {
					for (int i = 0; i < 1000; ++i) {
						slot = Arci<AtomicIntrusive>::create(t);
					}
				}
			);
		}

		for (auto& th : writers) { th.join(); }
		stop = true;
		for (auto& th : readers) { th.join(); }

		EXPECT_EQ(invalid, 0);
		EXPECT_EQ(semLive, 1);

		slot.reset();
		EXPECT_EQ(semLive, 0);
	}

	TEST_F(ArciSemantics, CopyOfCopyAndDestroyOrder) {
		auto a = Arci<Guarded>::create(1);
		auto b = a;
		auto c = b;
		EXPECT_EQ(a.ref_count(), 3u);
		EXPECT_EQ(c.get(), a.get());

		// Dropping the origin first keeps the object alive through the copies.
		a.reset();
		EXPECT_EQ(b.ref_count(), 2u);
		EXPECT_EQ(guardedDestroyed, 0);

		auto d = std::move(c);
		auto e = d;
		b.reset();
		EXPECT_EQ(e.ref_count(), 2u);
		EXPECT_TRUE(e->valid());

		e.reset();
		EXPECT_TRUE(d.ref_unique());
		EXPECT_EQ(guardedDestroyed, 0);

		d.reset();
		EXPECT_EQ(guardedCreated, 1);
		EXPECT_EQ(guardedDestroyed, 1);
	}

	TEST_F(ArciSemantics, NullHandling) {
		Arci<Guarded> empty {};
		Arci<Guarded> fromNull { nullptr };

		EXPECT_TRUE(empty == nullptr);
		EXPECT_TRUE(empty.ref_unique());
		EXPECT_EQ(empty.ref_count(), 0u);
		EXPECT_TRUE(empty == fromNull);

		auto copy = empty;
		auto moved = std::move(copy);
		EXPECT_TRUE(copy == nullptr);
		EXPECT_TRUE(moved == nullptr);

		moved = std::move(empty);
		moved.reset();
		moved.reset();
		EXPECT_TRUE(moved == nullptr);

		auto converted = std::move(moved).into<const Guarded>();
		EXPECT_TRUE(converted == nullptr);
		EXPECT_EQ(guardedCreated, 0);
	}

	TEST_F(ArciSemantics, PinFlushKeepsLiveCopies) {
		constexpr auto flushAt = Arci<Guarded>::pin_flush_at;
		auto slot = Arci<Guarded>::create(1);

		// Hold copies across the flush, so flushed credits and live words overlap.
		std::vector<Arci<Guarded>> held {};
		held.reserve(flushAt + 16);
		for (int i = 0; i < flushAt + 16; ++i) {
			held.push_back(slot);
		}
		EXPECT_EQ(slot.ref_count(), flushAt + 17u);

		for (int i = 0; i < 2 * flushAt; ++i) {
			auto transient = slot;
		}
		EXPECT_EQ(slot.ref_count(), flushAt + 17u);

		held.clear();
		EXPECT_TRUE(slot.ref_unique());
		EXPECT_EQ(guardedDestroyed, 0);

		slot.reset();
		EXPECT_EQ(guardedDestroyed, 1);
	}

	namespace {
		// Readers copy the shared slot until `stop`, checking every snapshot is a live object of an expected value.
		template <class Mutate_>
		void stressSharedSlot(ref<Arci<Guarded>> slot_, Mutate_&& mutate_, const int readers_ = 4) {
			std::atomic_bool stop { false };
			std::atomic_int invalid { 0 };

			std::vector<std::thread> readers {};
			for (int t = 0; t < readers_; ++t) {
				readers.emplace_back(
					[&] {
						while (not stop.load()) {
							auto snapshot = slot_;
							if (snapshot != nullptr && (not snapshot->valid() || snapshot->value < 1 || snapshot->value > 4)) {
								++invalid;
							}
							auto copyOfCopy = snapshot;
						}
					}
				);
			}

			mutate_();

			stop = true;
			for (auto& th : readers) { th.join(); }

			EXPECT_EQ(invalid, 0);
		}
	}

	TEST_F(ArciSemantics, ConcurrentMoveFromVsCopy) {
		Arci<Guarded> slot = Arci<Guarded>::create(1);

		stressSharedSlot(
			slot,
			[&slot] {
				std::vector<std::thread> movers {};
				for (int t = 1; t <= 2; ++t) {
					movers.emplace_back(
						[&slot, t] {
							for (int i = 0; i < 2000; ++i) {
								auto taken = std::move(slot);
								slot = Arci<Guarded>::create(t);
								auto again = std::move(slot).into<const Guarded>();
							}
						}
					);
				}
				for (auto& th : movers) { th.join(); }
			}
		);

		slot.reset();
		EXPECT_EQ(guardedCreated, guardedDestroyed);
	}

	TEST_F(ArciSemantics, ConcurrentResetVsCopy) {
		Arci<Guarded> slot = Arci<Guarded>::create(1);

		stressSharedSlot(
			slot,
			[&slot] {
				std::vector<std::thread> writers {};
				for (int t = 1; t <= 2; ++t) {
					writers.emplace_back(
						[&slot, t] {
							for (int i = 0; i < 2000; ++i) {
								slot.reset();
								slot = Arci<Guarded>::create(t + 2);
							}
						}
					);
				}
				for (auto& th : writers) { th.join(); }
			}
		);

		slot.reset();
		EXPECT_EQ(guardedCreated, guardedDestroyed);
	}

	TEST_F(ArciSemantics, ConcurrentWritersVsCopyFreeEachObjectOnce) {
		Arci<Guarded> slot = Arci<Guarded>::create(1);

		stressSharedSlot(
			slot,
			[&slot] {
				std::vector<std::thread> writers {};
				for (int t = 1; t <= 4; ++t) {
					writers.emplace_back(
						[&slot, t] {
							auto own = Arci<Guarded>::create(t);
							for (int i = 0; i < 2000; ++i) {
								auto keep = slot;
								slot = std::move(own);
								own = Arci<Guarded>::create(t);
							}
						}
					);
				}
				for (auto& th : writers) { th.join(); }
			},
			8
		);

		EXPECT_EQ(slot.ref_count(), 1u);
		EXPECT_EQ(guardedCreated, guardedDestroyed + 1);
		slot.reset();
		EXPECT_EQ(guardedCreated, guardedDestroyed);
	}

	/* Arci<const T> */

	static_assert(std::same_as<decltype(std::declval<const Arci<const AtomicIntrusive>&>().get()), const AtomicIntrusive*>);
	static_assert(std::is_convertible_v<Arci<AtomicIntrusive>, Arci<const AtomicIntrusive>>);
	static_assert(std::is_convertible_v<const Arci<AtomicIntrusive>&, Arci<const AtomicIntrusive>>);
	static_assert(not std::is_convertible_v<Arci<const AtomicIntrusive>, Arci<AtomicIntrusive>>);
	static_assert(not std::is_constructible_v<Arci<AtomicIntrusive>, Arci<const AtomicIntrusive>>);

	template <class From_, class To_>
	concept ArciInto = requires(From_ from_) { std::move(from_).template into<To_>(); };

	static_assert(ArciInto<Arci<AtomicIntrusive>, const AtomicIntrusive>);
	static_assert(not ArciInto<Arci<const AtomicIntrusive>, AtomicIntrusive>);

	TEST_F(ArciSemantics, ConstCreateAndDestroy) {
		{
			auto obj = Arci<const AtomicIntrusive>::create(5);
			EXPECT_EQ(obj->value, 5);
			EXPECT_EQ((*obj).value, 5);
			EXPECT_EQ(obj.ref_count(), 1);
			EXPECT_TRUE(obj.ref_unique());
			EXPECT_EQ(semLive, 1);
		}
		EXPECT_EQ(semLive, 0);
	}

	TEST_F(ArciSemantics, ConstCopyMoveReset) {
		auto a = Arci<const AtomicIntrusive>::create(6);
		auto b = a;
		EXPECT_EQ(a.ref_count(), 2);
		EXPECT_EQ(a.get(), b.get());

		auto c = std::move(b);
		EXPECT_TRUE(b == nullptr);
		EXPECT_EQ(c.ref_count(), 2);

		Arci<const AtomicIntrusive> d {};
		d = std::move(c);
		EXPECT_EQ(d->value, 6);

		a.reset();
		EXPECT_EQ(d.ref_count(), 1);
		EXPECT_EQ(semLive, 1);

		d.reset();
		EXPECT_EQ(semLive, 0);
	}

	TEST_F(ArciSemantics, MutableConvertsToConstByCopy) {
		auto src = Arci<AtomicIntrusive>::create(7);
		Arci<const AtomicIntrusive> view = src;

		EXPECT_EQ(view.get(), src.get());
		EXPECT_EQ(src.ref_count(), 2);

		src.reset();
		EXPECT_EQ(view->value, 7);
		EXPECT_EQ(semLive, 1);

		view.reset();
		EXPECT_EQ(semLive, 0);
	}

	TEST_F(ArciSemantics, MutableConvertsToConstByMove) {
		auto src = Arci<AtomicIntrusive>::create(8);
		const auto* const raw = src.get();
		Arci<const AtomicIntrusive> dst = std::move(src);

		EXPECT_TRUE(src == nullptr);
		EXPECT_EQ(dst.get(), raw);
		EXPECT_EQ(dst.ref_count(), 1);

		dst.reset();
		EXPECT_EQ(semLive, 0);
	}

	TEST_F(ArciSemantics, IntoConstKeepsObject) {
		auto src = Arci<AtomicIntrusive>::create(9);
		auto dst = src.into<const AtomicIntrusive>();

		EXPECT_EQ(src.ref_count(), 2);
		EXPECT_EQ(dst->value, 9);
	}

	TEST_F(ArciSemantics, ConstFromThisCountsOnSharedObject) {
		auto src = Arci<AtomicIntrusive>::create(10);
		Arci<const AtomicIntrusive> other = src->arci_from_this<const AtomicIntrusive>();

		EXPECT_EQ(src.ref_count(), 2);
		EXPECT_EQ(other.get(), src.get());
	}

	/* Arc */

	TEST_F(ArcSemantics, ReadAfterMove) {
		auto src = make_smr<Base>(11);
		auto dst = std::move(src);

		EXPECT_TRUE(src == nullptr);
		EXPECT_TRUE(src.empty());
		EXPECT_EQ(src.ctrlRefs(), 0);
		EXPECT_EQ(dst->value, 11);
		EXPECT_EQ(semLive, 1);
	}

	TEST_F(ArcSemantics, MovedFromIsReassignable) {
		auto src = make_smr<Base>(1);
		auto dst = std::move(src);

		src = make_smr<Base>(2);
		EXPECT_EQ(src->value, 2);
		EXPECT_EQ(dst->value, 1);
		EXPECT_EQ(semLive, 2);
	}

	TEST_F(ArcSemantics, CopyAssignReleasesPreviousTarget) {
		auto a = make_smr<Base>(1);
		auto b = make_smr<Base>(2);

		a = b;

		EXPECT_EQ(semLive, 1);
		EXPECT_EQ(a->value, 2);
		EXPECT_TRUE(a == b);
		EXPECT_EQ(b.ctrlRefs(), 2);
	}

	// Note: same-type move assignment swaps, so the previous target lives on in the moved-from operand.
	TEST_F(ArcSemantics, MoveAssignSwapsWithSource) {
		auto a = make_smr<Base>(1);
		auto b = make_smr<Base>(2);

		a = std::move(b);

		EXPECT_EQ(a->value, 2);
		EXPECT_EQ(b->value, 1);
		EXPECT_EQ(semLive, 2);

		b.reset();
		EXPECT_EQ(semLive, 1);
	}

	TEST_F(ArcSemantics, SelfAssignKeepsObject) {
		auto a = make_smr<Base>(5);
		auto& alias = a;

		a = alias;
		EXPECT_EQ(a->value, 5);
		EXPECT_EQ(semLive, 1);
	}

	TEST_F(ArcSemantics, CopiesShareObject) {
		auto a = make_smr<Base>(3);
		auto b = a;

		b->value = 4;
		EXPECT_EQ(a->value, 4);
		EXPECT_TRUE(a == b);
		EXPECT_EQ(a.get(), b.get());
	}

	TEST_F(ArcSemantics, ResetOneCopyKeepsOthersAlive) {
		auto a = make_smr<Base>(3);
		auto b = a;

		a.reset();

		EXPECT_TRUE(a.empty());
		EXPECT_EQ(semLive, 1);
		EXPECT_EQ(b->value, 3);
		EXPECT_EQ(b.ctrlRefs(), 1);
	}

	TEST_F(ArcSemantics, SwapExchangesTargets) {
		auto a = make_smr<Base>(1);
		auto b = make_smr<Base>(2);

		a.swap(b);

		EXPECT_EQ(a->value, 2);
		EXPECT_EQ(b->value, 1);
		EXPECT_EQ(semLive, 2);
	}

	TEST_F(ArcSemantics, CopyOfEmptyStaysEmpty) {
		const Arc<Base> empty {};
		auto copy = empty;

		EXPECT_TRUE(copy.empty());
		EXPECT_EQ(copy.ctrlRefs(), 0);
	}

	TEST_F(ArcSemantics, MoveConvertsToBase) {
		auto derived = make_smr<Derived>(8);
		Arc<Base> base { std::move(derived) };

		EXPECT_TRUE(derived.empty());
		EXPECT_EQ(base->value, 8);

		base.reset();
		EXPECT_EQ(semLive, 0);
	}

	TEST_F(ArcSemantics, ConcurrentCopyReadOfSharedSource) {
		const auto shared = make_smr<Base>(42);

		std::atomic_int mismatches { 0 };
		std::vector<std::thread> pool {};
		for (int t = 0; t < 8; ++t) {
			pool.emplace_back(
				[&] {
					for (int i = 0; i < 2000; ++i) {
						auto local = shared;
						if (local->value != 42) {
							++mismatches;
						}
					}
				}
			);
		}
		for (auto& th : pool) { th.join(); }

		EXPECT_EQ(mismatches, 0);
		EXPECT_EQ(shared.ctrlRefs(), 1);
		EXPECT_EQ(semLive, 1);
	}

	TEST_F(ArcSemantics, ConcurrentCopyAssignToPrivateTargets) {
		const auto shared = make_smr<Base>(42);

		std::vector<std::thread> pool {};
		for (int t = 0; t < 8; ++t) {
			pool.emplace_back(
				[&] {
					Arc<Base> own {};
					for (int i = 0; i < 2000; ++i) {
						own = shared;
					}
					EXPECT_EQ(own->value, 42);
				}
			);
		}
		for (auto& th : pool) { th.join(); }

		EXPECT_EQ(shared.ctrlRefs(), 1);
		EXPECT_EQ(semLive, 1);
	}

	TEST_F(ArcSemantics, ConcurrentCreateAndDestroy) {
		std::vector<std::thread> pool {};
		for (int t = 0; t < 8; ++t) {
			pool.emplace_back(
				[t] {
					for (int i = 0; i < 2000; ++i) {
						auto local = make_smr<Base>(t);
						auto moved = std::move(local);
					}
				}
			);
		}
		for (auto& th : pool) { th.join(); }

		EXPECT_EQ(semLive, 0);
	}
}
