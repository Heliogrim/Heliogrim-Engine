#include "pch.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <set>
#include <Engine.ACS/Archetype/Layout.hpp>
#include <Engine.ACS/Archetype/LayoutInfo.hpp>

using namespace hg;
using namespace hg::engine::acs;

namespace Acs {
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

		struct alignas(16) TestActor {
			char data[40];
		};

		using SingleInfo = LayoutInfo<ActorLayout<Slot<"body", A>>, TestActor>;
		using NestedInfo = LayoutInfo<
			ActorLayout<Slot<"body", A, Children<Slot<"wheels", B, Count<4>, Children<Slot<"hub", C>>>>>>,
			TestActor
		>;

		constexpr u32 noParent = ~0u;
	}

	TEST(Layout, SingleRoot) {
		ASSERT_EQ(SingleInfo::entry_count, 1u);

		const auto& root = SingleInfo::entries[0];
		// 8 bytes of front padding for the 16-aligned actor; the root ends at the actor.
		EXPECT_EQ(root.offset, 8u);
		EXPECT_EQ(root.offset + root.size, SingleInfo::actor_offset);
		EXPECT_EQ(root.size, sizeof(A));
		EXPECT_EQ(root.parentIndex, noParent);
		EXPECT_EQ(root.arrayIndex, 0u);
	}

	TEST(Layout, NestedArray) {
		ASSERT_EQ(NestedInfo::entry_count, 1u + 4u + 4u);

		constexpr auto body = NestedInfo::index_of<"body">();
		EXPECT_EQ(NestedInfo::entries[body].parentIndex, noParent);

		std::set<u32> wheels {};
		std::set<u16> wheelElements {};
		for (u32 i = 0; i < NestedInfo::entry_count; ++i) {
			if (NestedInfo::entries[i].parentIndex == body) {
				wheels.insert(i);
				wheelElements.insert(NestedInfo::entries[i].arrayIndex);
			}
		}
		EXPECT_EQ(wheels.size(), 4uLL);
		EXPECT_EQ(wheelElements, (std::set<u16> { 0u, 1u, 2u, 3u }));

		std::set<u32> hubParents {};
		u32 hubCount = 0;
		for (u32 i = 0; i < NestedInfo::entry_count; ++i) {
			const auto parent = NestedInfo::entries[i].parentIndex;
			if (parent != noParent && parent != body) {
				++hubCount;
				hubParents.insert(parent);
				EXPECT_EQ(NestedInfo::entries[i].size, sizeof(C));
			}
		}
		EXPECT_EQ(hubCount, 4u);
		EXPECT_EQ(hubParents, wheels);

		/* Alignment per entry type, all inside the block, no overlap. */

		const std::array<std::size_t, 3> aligns { alignof(A), alignof(B), alignof(C) };
		const auto alignOf = [&](const auto& entry_) {
			return entry_.parentIndex == noParent ? aligns[0] : entry_.parentIndex == body ? aligns[1] : aligns[2];
		};

		for (u32 i = 0; i < NestedInfo::entry_count; ++i) {
			const auto& entry = NestedInfo::entries[i];
			EXPECT_EQ(entry.offset % alignOf(entry), 0u) << i;
			EXPECT_LE(entry.offset + entry.size, NestedInfo::block_size) << i;

			for (u32 j = i + 1; j < NestedInfo::entry_count; ++j) {
				const auto& other = NestedInfo::entries[j];
				const bool disjoint = entry.offset + entry.size <= other.offset || other.offset + other.size <= entry.offset;
				EXPECT_TRUE(disjoint) << i << " overlaps " << j;
			}
		}

		/* One type index per distinct component type. */

		std::set<u32> typeIndices {};
		for (const auto& entry : NestedInfo::entries) { typeIndices.insert(entry.typeIndex); }
		EXPECT_EQ(typeIndices, (std::set<u32> { 0u, 1u, 2u }));
	}

	TEST(Layout, PathLookup) {
		EXPECT_EQ(NestedInfo::index_of<"body/wheels">(), NestedInfo::index_of<"wheels">());
		EXPECT_EQ(NestedInfo::index_of<"body/wheels/hub">(), NestedInfo::index_of<"hub">());
		EXPECT_EQ(NestedInfo::index_of<"wheels/hub">(), NestedInfo::index_of<"hub">());

		constexpr auto body = NestedInfo::index_of<"body">();
		constexpr auto wheels = NestedInfo::index_of<"wheels">();
		constexpr auto hub = NestedInfo::index_of<"hub">();

		EXPECT_EQ(NestedInfo::entries[wheels].parentIndex, body);
		EXPECT_EQ(NestedInfo::entries[wheels].arrayIndex, 0u);
		EXPECT_EQ(NestedInfo::entries[hub].parentIndex, wheels);
	}

	TEST(Layout, PathDisambiguation) {
		using Info = LayoutInfo<
			ActorLayout<Slot<"r", A, Children<Slot<"p", B, Children<Slot<"x", C>>>, Slot<"q", B, Children<Slot<"x", C>>>>>>,
			TestActor
		>;

		constexpr auto p = Info::index_of<"p">();
		constexpr auto q = Info::index_of<"q">();
		constexpr auto px = Info::index_of<"p/x">();
		constexpr auto qx = Info::index_of<"q/x">();

		EXPECT_NE(px, qx);
		EXPECT_EQ(Info::entries[px].parentIndex, p);
		EXPECT_EQ(Info::entries[qx].parentIndex, q);
		EXPECT_EQ((std::set<u32> { px, qx }), (std::set<u32> { Info::index_of<"r/p/x">(), Info::index_of<"r/q/x">() }));
	}

	TEST(Layout, Bitmap) {
		// Fits in one word (16 bytes at 8-byte min align); nested block spans 128 bytes at 1-byte min align.
		EXPECT_TRUE((std::same_as<SingleInfo::bitmap_type, u64>));
		EXPECT_TRUE((std::same_as<NestedInfo::bitmap_type, std::array<u64, 2>>));

		std::set<u32> offsets {};
		for (const auto& entry : NestedInfo::entries) { offsets.insert(entry.offset); }

		u32 setCount = 0;
		for (u32 offset = 0; offset < NestedInfo::block_size; offset += NestedInfo::min_align) {
			const bool set = NestedInfo::is_start(offset);
			EXPECT_EQ(set, offsets.contains(offset)) << offset;
			setCount += set ? 1u : 0u;
		}
		EXPECT_EQ(setCount, NestedInfo::entry_count);

		// Front padding is no component start.
		EXPECT_FALSE(SingleInfo::is_start(0u));
		EXPECT_TRUE(SingleInfo::is_start(8u));
	}

	TEST(Layout, ActorAfterBlock) {
		EXPECT_GE(SingleInfo::actor_offset, SingleInfo::block_size);
		EXPECT_EQ(SingleInfo::slot_stride % alignof(TestActor), 0u);
		EXPECT_GE(SingleInfo::slot_stride, SingleInfo::actor_offset + sizeof(TestActor));

		EXPECT_GE(NestedInfo::actor_offset, NestedInfo::block_size);
		EXPECT_EQ(NestedInfo::actor_offset % alignof(TestActor), 0u);
		EXPECT_EQ(NestedInfo::slot_stride % alignof(TestActor), 0u);
		EXPECT_EQ(NestedInfo::slot_stride % NestedInfo::block_align, 0u);
		EXPECT_GE(NestedInfo::slot_stride, NestedInfo::actor_offset + sizeof(TestActor));
	}
}
