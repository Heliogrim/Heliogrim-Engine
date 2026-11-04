#include "pch.h"

#include <algorithm>
#include <set>
#include <Engine.ACS/Archetype/Layout.hpp>
#include <Engine.ACS/Archetype/LayoutInfo.hpp>
#include <Engine.ACS/Archetype/Overridable.hpp>

using namespace hg;
using namespace hg::engine::acs;

namespace AcsExtend {
	namespace {
		struct A {
			double value;
		};

		struct B {
			int value;
		};

		struct C {
			char value[24];
		};

		struct SameC : C {};

		struct LargerC : C {
			char extra[40];
		};

		struct alignas(16) TestActor {
			char data[40];
		};

		struct LargerActor : TestActor {
			char more[56];
		};

		using BaseLayout = ActorLayout<Slot<"body", A, Children<Slot<"wheels", B, Count<4>, Children<Slot<"hub", C>>>>>>;
		using BaseInfo = LayoutInfo<BaseLayout, TestActor>;

		template <typename Base_, typename Derived_, fixed_string Path_>
		void expectSameActorRelative() {
			EXPECT_EQ(
				Base_::actor_relative(Base_::template index_of<Path_>()),
				Derived_::actor_relative(Derived_::template index_of<Path_>())
			) << Path_.view();
		}

		// Every base entry keeps its distance to the actor (as a multiset, independent of entry order).
		template <typename Base_, typename Derived_>
		void expectBaseKept() {
			expectSameActorRelative<Base_, Derived_, "body">();
			expectSameActorRelative<Base_, Derived_, "body/wheels">();
			expectSameActorRelative<Base_, Derived_, "body/wheels/hub">();

			std::multiset<u32> base {};
			for (u32 i = 0; i < Base_::entry_count; ++i) { base.insert(Base_::actor_relative(i)); }
			std::multiset<u32> derived {};
			for (u32 i = 0; i < Derived_::entry_count; ++i) { derived.insert(Derived_::actor_relative(i)); }

			EXPECT_TRUE(std::ranges::includes(derived, base));
		}

		template <typename Info_>
		void expectDisjoint() {
			for (u32 i = 0; i < Info_::entry_count; ++i) {
				const auto& entry = Info_::entries[i];
				EXPECT_LE(entry.offset + entry.size, Info_::block_size) << i;

				for (u32 j = i + 1; j < Info_::entry_count; ++j) {
					const auto& other = Info_::entries[j];
					const bool disjoint = entry.offset + entry.size <= other.offset || other.offset + other.size <= entry.offset;
					EXPECT_TRUE(disjoint) << i << " overlaps " << j;
				}
			}
		}

		template <typename Info_>
		std::set<u32> childrenOf(const u32 parent_) {
			std::set<u32> result {};
			for (u32 i = 0; i < Info_::entry_count; ++i) {
				if (Info_::entries[i].parentIndex == parent_) {
					result.insert(i);
				}
			}
			return result;
		}
	}

	TEST(Extend, AppendKeepsBaseActorRelativeOffsets) {
		using Derived = Extend<BaseLayout, Slot<"turret", A>, Slot<"radio", B, Count<2>>>;
		using DerivedInfo = LayoutInfo<Derived, TestActor>;
		using LargerInfo = LayoutInfo<Derived, LargerActor>;
		using ChainInfo = LayoutInfo<Extend<Derived, Slot<"gun", C>>, LargerActor>;

		expectBaseKept<BaseInfo, DerivedInfo>();
		expectBaseKept<BaseInfo, LargerInfo>();
		expectBaseKept<BaseInfo, ChainInfo>();
		expectSameActorRelative<DerivedInfo, ChainInfo, "turret">();
		expectSameActorRelative<DerivedInfo, ChainInfo, "radio">();

		ASSERT_EQ(DerivedInfo::entry_count, BaseInfo::entry_count + 1u + 2u);
		ASSERT_EQ(ChainInfo::entry_count, DerivedInfo::entry_count + 1u);

		constexpr auto body = DerivedInfo::index_of<"body">();
		constexpr auto turret = DerivedInfo::index_of<"body/turret">();
		EXPECT_EQ(DerivedInfo::entries[turret].parentIndex, body);
		EXPECT_EQ(DerivedInfo::entries[turret].size, sizeof(A));

		const auto rootChildren = childrenOf<DerivedInfo>(body);
		EXPECT_EQ(rootChildren.size(), 4uLL + 1uLL + 2uLL);
		EXPECT_TRUE(rootChildren.contains(DerivedInfo::index_of<"radio">()));
		EXPECT_EQ(ChainInfo::entries[ChainInfo::index_of<"gun">()].parentIndex, ChainInfo::index_of<"body">());

		expectDisjoint<DerivedInfo>();
		expectDisjoint<ChainInfo>();

		EXPECT_EQ(LargerInfo::actor_offset % alignof(LargerActor), 0u);
		EXPECT_GE(LargerInfo::slot_stride, LargerInfo::actor_offset + sizeof(LargerActor));
	}

	namespace {
		struct BaseActor {
			double value;
		};

		struct alignas(16) Actor16 : BaseActor {};

		struct alignas(64) Actor64 : BaseActor {};

		// 24 byte block (A at 0, A at 8, B at 16, padded to 8): no multiple of 16 or 64.
		using SmallLayout = ActorLayout<Slot<"r", A, Children<Slot<"x", A>, Slot<"y", B>>>>;
		using SmallInfo = LayoutInfo<SmallLayout, BaseActor>;

		template <typename Layout_, typename Actor_>
		void expectSmallBaseKept() {
			using Derived = LayoutInfo<Layout_, Actor_>;
			expectSameActorRelative<SmallInfo, Derived, "r">();
			expectSameActorRelative<SmallInfo, Derived, "r/x">();
			expectSameActorRelative<SmallInfo, Derived, "r/y">();

			std::multiset<u32> base {};
			for (u32 i = 0; i < SmallInfo::entry_count; ++i) { base.insert(SmallInfo::actor_relative(i)); }
			std::multiset<u32> derived {};
			for (u32 i = 0; i < Derived::entry_count; ++i) { derived.insert(Derived::actor_relative(i)); }
			EXPECT_TRUE(std::ranges::includes(derived, base));

			EXPECT_EQ(Derived::actor_offset % alignof(Actor_), 0u);
			EXPECT_EQ(Derived::slot_stride % alignof(Actor_), 0u);
			expectDisjoint<Derived>();
		}
	}

	TEST(Extend, StricterDerivedActorKeepsBaseActorRelativeOffsets) {
		ASSERT_EQ(SmallInfo::block_size, 24u);
		EXPECT_EQ(SmallInfo::actor_relative(SmallInfo::index_of<"r">()), 24u);

		using Derived = Extend<SmallLayout, Slot<"z", B>>;

		// Reviewer case: block 24, base actor align 8, derived actor align 16.
		expectSmallBaseKept<Derived, Actor16>();
		EXPECT_EQ((LayoutInfo<Derived, Actor16>::actor_relative(LayoutInfo<Derived, Actor16>::index_of<"r">())), 24u);

		expectSmallBaseKept<Derived, Actor64>();

		// Only the actor gets stricter, no added slots.
		expectSmallBaseKept<SmallLayout, Actor64>();
	}

	TEST(Extend, ChildUnderNamedSlot) {
		using DerivedInfo = LayoutInfo<Extend<BaseLayout, At<"wheels", Slot<"light", B>>>, TestActor>;

		expectBaseKept<BaseInfo, DerivedInfo>();
		ASSERT_EQ(DerivedInfo::entry_count, BaseInfo::entry_count + 4u);

		EXPECT_EQ(DerivedInfo::index_of<"light">(), DerivedInfo::index_of<"body/wheels/light">());
		EXPECT_EQ(DerivedInfo::index_of<"wheels/light">(), DerivedInfo::index_of<"light">());
		EXPECT_EQ(DerivedInfo::entries[DerivedInfo::index_of<"light">()].parentIndex, DerivedInfo::index_of<"wheels">());

		// One light per wheel element.
		const auto wheels = childrenOf<DerivedInfo>(DerivedInfo::index_of<"body">());
		ASSERT_EQ(wheels.size(), 4uLL);

		std::set<u32> lightParents {};
		u32 lightCount = 0;
		for (const auto wheel : wheels) {
			for (const auto child : childrenOf<DerivedInfo>(wheel)) {
				if (DerivedInfo::entries[child].size == sizeof(B)) {
					++lightCount;
					lightParents.insert(wheel);
				}
			}
		}
		EXPECT_EQ(lightCount, 4u);
		EXPECT_EQ(lightParents, wheels);

		expectDisjoint<DerivedInfo>();
	}

	TEST(Override, SameSizeKeepsOffsets) {
		using DerivedInfo = LayoutInfo<Extend<BaseLayout, Override<"hub", SameC>>, TestActor>;

		expectBaseKept<BaseInfo, DerivedInfo>();
		EXPECT_EQ(DerivedInfo::entry_count, BaseInfo::entry_count);
		EXPECT_EQ(DerivedInfo::block_size, BaseInfo::block_size);
		EXPECT_EQ(DerivedInfo::entries[DerivedInfo::index_of<"hub">()].size, sizeof(SameC));

		// Overrides reach slots of an extended base, appended or original.
		using Turreted = Extend<BaseLayout, Slot<"turret", C>>;
		using TurretedInfo = LayoutInfo<Turreted, TestActor>;
		using OverriddenInfo = LayoutInfo<
			Extend<Turreted, Override<"turret", SameC>, Override<"wheels/hub", SameC>>,
			TestActor
		>;

		expectBaseKept<BaseInfo, OverriddenInfo>();
		expectSameActorRelative<TurretedInfo, OverriddenInfo, "turret">();
		EXPECT_EQ(OverriddenInfo::entry_count, TurretedInfo::entry_count);
	}

	TEST(Override, LargerOnOverridableBase) {
		using OverridableInfo = LayoutInfo<Overridable<BaseLayout>, TestActor>;
		using DerivedInfo = LayoutInfo<Extend<Overridable<BaseLayout>, Override<"hub", LargerC>>, TestActor>;

		EXPECT_TRUE(overridable_v<Overridable<BaseLayout>>);
		EXPECT_FALSE(overridable_v<BaseLayout>);
		expectBaseKept<BaseInfo, OverridableInfo>();

		EXPECT_EQ(DerivedInfo::entry_count, BaseInfo::entry_count);
		EXPECT_EQ(DerivedInfo::entries[DerivedInfo::index_of<"hub">()].size, sizeof(LargerC));
		EXPECT_NE(
			DerivedInfo::actor_relative(DerivedInfo::index_of<"body">()),
			BaseInfo::actor_relative(BaseInfo::index_of<"body">())
		);
		expectDisjoint<DerivedInfo>();

		/*
		 * Compile-fail case, checked manually: the same override on a base without the Overridable marker.
		 *
		 * using BrokenInfo = LayoutInfo<Extend<BaseLayout, Override<"hub", LargerC>>, TestActor>;
		 * constexpr auto broken = BrokenInfo::block_size;
		 *
		 * Expected: static_assert "Override changes a base slot's actor-relative offset: the base must provide
		 * deducing-this bodies (Overridable)." in the instantiation of Override<"hub", LargerC>.
		 */
	}
}
