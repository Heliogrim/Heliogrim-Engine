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

		struct SemanticsFixture :
			public ::testing::Test {
			void SetUp() override { semLive = 0; }
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

	// Defect: a reader copying a slot while writers move-assign it corrupts the heap / double-destroys (observed invalid reads, semLive == -1, CRT heap assert).
	TEST_F(ArciSemantics, DISABLED_ConcurrentReadersAndWritersOfSharedSlot) {
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
