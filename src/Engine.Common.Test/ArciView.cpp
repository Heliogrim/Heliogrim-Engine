#include "pch.h"

#include <atomic>
#include <thread>
#include <type_traits>
#include <vector>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Managed/AtomicRefCountedIntrusive.hpp>

using namespace hg;

namespace Common {
	namespace {
		inline std::atomic_int viewCreated { 0 };
		inline std::atomic_int viewDestroyed { 0 };

		// Detects use after free (magic cleared on destroy) and double free (destroy count).
		struct ViewGuarded :
			public ArcFromThis<ViewGuarded> {
			static constexpr int alive = 0x5AFE;

			int magic = alive;
			int value;

			explicit ViewGuarded(const int value_ = 0) :
				value(value_) { ++viewCreated; }

			~ViewGuarded() {
				magic = 0;
				++viewDestroyed;
			}

			[[nodiscard]] bool valid() const { return magic == alive; }
		};

		struct ViewFixture :
			public ::testing::Test {
			void SetUp() override {
				viewCreated = 0;
				viewDestroyed = 0;
			}
		};

		template <class View_>
		concept ViewOf = requires(const Arci<ViewGuarded>& a) { a.template view<View_>(); };

		template <class View_>
		concept ConstViewOf = requires(const Arci<const ViewGuarded>& a) { a.template view<View_>(); };
	}

	using ArciView = ViewFixture;

	using MutableView = Arci<ViewGuarded>::View<ViewGuarded>;
	using ConstView = Arci<ViewGuarded>::View<const ViewGuarded>;
	using ConstSourceView = Arci<const ViewGuarded>::View<const ViewGuarded>;

	// Access matrix: Arci<T> gives View<T> and View<const T>, Arci<const T> gives View<const T> only.
	static_assert(ViewOf<ViewGuarded>);
	static_assert(ViewOf<const ViewGuarded>);
	static_assert(ConstViewOf<const ViewGuarded>);
	static_assert(not ConstViewOf<ViewGuarded>);
	static_assert(std::same_as<decltype(std::declval<MutableView&>().get()), ViewGuarded*>);
	static_assert(std::same_as<decltype(std::declval<ConstView&>().get()), const ViewGuarded*>);

	// Not copyable, not movable, no heap allocation.
	static_assert(not std::is_copy_constructible_v<MutableView>);
	static_assert(not std::is_move_constructible_v<MutableView>);
	static_assert(not std::is_copy_assignable_v<ConstView>);
	static_assert(not std::is_move_assignable_v<ConstView>);
	template <class View_>
	concept HeapAllocatable = requires { new View_ { std::declval<const Arci<ViewGuarded>&>() }; };
	static_assert(not HeapAllocatable<MutableView>);
	static_assert(not HeapAllocatable<ConstView>);

	TEST_F(ArciView, KeepsObjectAliveAfterSlotReset) {
		auto slot = Arci<ViewGuarded>::create(7);
		{
			auto view = slot.view();
			slot.reset();
			EXPECT_EQ(viewDestroyed, 0);
			EXPECT_TRUE(view->valid());
			EXPECT_EQ(view->value, 7);
		}
		EXPECT_EQ(viewCreated, viewDestroyed);
	}

	TEST_F(ArciView, GetMatchesSlotAndConstAccess) {
		auto slot = Arci<ViewGuarded>::create(1);
		auto view = slot.view();
		auto constView = slot.view<const ViewGuarded>();
		auto fromConst = Arci<const ViewGuarded> { slot }.view();

		EXPECT_EQ(view.get(), slot.get());
		EXPECT_EQ(constView.get(), slot.get());
		EXPECT_EQ(fromConst.get(), slot.get());
		EXPECT_EQ(slot.ref_count(), 4u);
	}

	namespace {
		inline std::atomic_size_t viewRmws { 0 };

		template <class Ux_>
		struct RmwCountingAtomic {
			std::atomic<Ux_> value;

			constexpr RmwCountingAtomic(Ux_ value_) noexcept :
				value(value_) {}

			Ux_ load(std::memory_order order_ = std::memory_order::seq_cst) const noexcept { return value.load(order_); }

			Ux_ fetch_add(Ux_ arg_, std::memory_order order_ = std::memory_order::seq_cst) noexcept {
				++viewRmws;
				return value.fetch_add(arg_, order_);
			}

			Ux_ exchange(Ux_ value_, std::memory_order order_ = std::memory_order::seq_cst) noexcept {
				++viewRmws;
				return value.exchange(value_, order_);
			}

			bool compare_exchange_strong(Ux_& expected_, Ux_ desired_, std::memory_order s_, std::memory_order f_) noexcept {
				++viewRmws;
				return value.compare_exchange_strong(expected_, desired_, s_, f_);
			}
		};

		struct RmwPolicy {
			template <class Ux_>
			using atomic = RmwCountingAtomic<Ux_>;

			static void fence(std::memory_order order_) noexcept { std::atomic_thread_fence(order_); }
		};

		struct RmwObject :
			public ArcFromThis<RmwObject, RmwPolicy> {};
	}

	TEST_F(ArciView, CreationCostsExactlyTheCopyPath) {
		auto slot = Arci<RmwObject, RmwPolicy>::create();

		viewRmws = 0;
		{ auto copy = slot; }
		const auto copyAndDestroy = viewRmws.load();
		EXPECT_EQ(copyAndDestroy, 3u);

		viewRmws = 0;
		{ auto view = slot.view(); }
		EXPECT_EQ(viewRmws.load(), copyAndDestroy);

		viewRmws = 0;
		{ auto view = slot.view<const RmwObject>(); }
		EXPECT_EQ(viewRmws.load(), copyAndDestroy);
	}

	TEST_F(ArciView, DestroyReleasesReference) {
		auto slot = Arci<ViewGuarded>::create(1);
		{
			auto view = slot.view();
			EXPECT_EQ(slot.ref_count(), 2u);
		}
		EXPECT_EQ(slot.ref_count(), 1u);
		slot.reset();
		EXPECT_EQ(viewCreated, viewDestroyed);
	}

	TEST_F(ArciView, NullSlotGivesNullView) {
		Arci<ViewGuarded> slot {};
		auto view = slot.view();
		EXPECT_EQ(view.get(), nullptr);
	}

	TEST_F(ArciView, ConcurrentWritersWhileHeld) {
		auto slot = Arci<ViewGuarded>::create(1);
		std::atomic_bool stop { false };
		std::atomic_int invalid { 0 };

		std::vector<std::thread> readers {};
		for (int t = 0; t < 4; ++t) {
			readers.emplace_back(
				[&] {
					while (not stop.load()) {
						auto view = slot.view();
						if (view.get() != nullptr) {
							// Writers may release the slot's reference meanwhile; the View keeps the object alive.
							for (int i = 0; i < 16; ++i) {
								if (not view->valid() || view->value < 1 || view->value > 3) { ++invalid; }
							}
						}
					}
				}
			);
		}

		std::vector<std::thread> writers {};
		for (int t = 1; t <= 2; ++t) {
			writers.emplace_back(
				[&slot, t] {
					for (int i = 0; i < 2000; ++i) {
						slot = Arci<ViewGuarded>::create(t + 1);
						if (i % 7 == 0) { slot.reset(); }
					}
				}
			);
		}
		for (auto& th : writers) { th.join(); }

		stop = true;
		for (auto& th : readers) { th.join(); }

		EXPECT_EQ(invalid, 0);
		slot.reset();
		EXPECT_EQ(viewCreated, viewDestroyed);
	}
}
