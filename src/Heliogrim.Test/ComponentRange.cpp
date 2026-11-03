#include "pch.h"

/**/

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <set>
#include <span>
#include <vector>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Heliogrim/Actor/ComponentRange.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>

using namespace hg;

namespace HeliogrimApi {
	namespace {
		[[nodiscard]] HierarchyComponent make() {
			return HierarchyComponent {
				generate_component_guid(),
				CachedActorPointer { .guid = invalid_actor_guid, .cached = nullptr },
				nullptr
			};
		}

		/* Stand-in for an archetype slot: components first, the actor at the end of the component block. */
		struct FakeSlot {
			alignas(16) HierarchyComponent a = make();
			alignas(16) HierarchyComponent b = make();
			alignas(16) HierarchyComponent c = make();
			alignas(16) u64 actor = 0uLL;
		};

		constexpr u32 min_align = 16u;

		struct ArchetypeFixture {
			FakeSlot slot {};
			Vector<uintptr_t> offsets {};
			u64 bitmap = 0uLL;

			ArchetypeFixture() {
				for (const auto* const component : { &slot.a, &slot.b, &slot.c }) {
					offsets.emplace_back(offsetOf(component));
					bitmap |= 1uLL << (offsetOf(component) / min_align);
				}
			}

			[[nodiscard]] uintptr_t offsetOf(const ptr<const void> p_) const noexcept {
				return reinterpret_cast<uintptr_t>(p_) - reinterpret_cast<uintptr_t>(&slot);
			}

			[[nodiscard]] ComponentRange range() noexcept {
				return ComponentRange::archetype(
					reinterpret_cast<std::byte*>(&slot),
					offsets,
					&bitmap,
					static_cast<u32>(offsetOf(&slot.actor)),
					min_align
				);
			}
		};

		struct OtherBase {
			virtual ~OtherBase() = default;

			u64 payload[3] { 1uLL, 2uLL, 3uLL };
		};

		struct PolyComponent : public OtherBase, public HierarchyComponent {
			PolyComponent() :
				OtherBase(),
				HierarchyComponent(make()) {}
		};
	}

	TEST(ComponentRange, RangesConcepts) {
		static_assert(std::ranges::random_access_range<ComponentRange>);
		static_assert(std::ranges::sized_range<ComponentRange>);
		static_assert(std::ranges::view<ComponentRange>);
		static_assert(std::same_as<std::ranges::range_value_t<ComponentRange>, ptr<HierarchyComponent>>);
		SUCCEED();
	}

	TEST(ComponentRange, ArchetypeIteratesAll) {
		ArchetypeFixture fx {};
		const auto range = fx.range();

		const std::set<ptr<HierarchyComponent>> expect { &fx.slot.a, &fx.slot.b, &fx.slot.c };
		const std::set<ptr<HierarchyComponent>> actual { range.begin(), range.end() };

		EXPECT_EQ(range.size(), 3uLL);
		EXPECT_EQ(actual, expect);
		EXPECT_EQ(range[1], &fx.slot.b);
	}

	TEST(ComponentRange, ContainsArchetype) {
		ArchetypeFixture fx {};
		ArchetypeFixture other {};
		const auto range = fx.range();

		EXPECT_TRUE(range.contains(&fx.slot.a));
		EXPECT_TRUE(range.contains(&fx.slot.b));
		EXPECT_TRUE(range.contains(&fx.slot.c));

		EXPECT_FALSE(range.contains(&other.slot.a));
		EXPECT_FALSE(range.contains(reinterpret_cast<const HierarchyComponent*>(&fx.slot.actor)));
		EXPECT_FALSE(range.contains(nullptr));

		const auto* const interior = reinterpret_cast<const std::byte*>(&fx.slot.a);
		EXPECT_FALSE(range.contains(reinterpret_cast<const HierarchyComponent*>(interior + 8)));
		EXPECT_FALSE(range.contains(reinterpret_cast<const HierarchyComponent*>(interior + min_align)));
	}

	TEST(ComponentRange, DynamicMatchesCompactSet) {
		auto a = make();
		auto b = make();
		auto c = make();
		auto stranger = make();

		CompactSet<ptr<HierarchyComponent>> set {};
		set.insert(&a);
		set.insert(&b);
		set.insert(&c);

		const auto range = ComponentRange::dynamic(set);

		EXPECT_EQ(range.size(), set.size());
		EXPECT_TRUE(std::ranges::equal(range, set));
		for (const auto* const entry : set) {
			EXPECT_TRUE(range.contains(entry));
		}
		EXPECT_FALSE(range.contains(&stranger));

		const CompactSet<ptr<HierarchyComponent>> none {};
		EXPECT_TRUE(ComponentRange::dynamic(none).empty());
	}

	TEST(ComponentRange, PolymorphicSubobject) {
		alignas(16) PolyComponent poly {};
		const auto base = reinterpret_cast<uintptr_t>(&poly);
		const auto* const subobject = static_cast<HierarchyComponent*>(&poly);
		ASSERT_NE(reinterpret_cast<uintptr_t>(subobject), base);

		const Vector<uintptr_t> offsets { reinterpret_cast<uintptr_t>(subobject) - base };
		ASSERT_EQ(offsets.front() % min_align, 0uLL) << "fixture assumes the subobject lands on the minimum alignment";
		const u64 bitmap = 1uLL << (offsets.front() / min_align);

		const auto range = ComponentRange::archetype(
			reinterpret_cast<std::byte*>(&poly),
			offsets,
			&bitmap,
			static_cast<u32>(sizeof(PolyComponent)),
			min_align
		);

		ASSERT_EQ(range.size(), 1uLL);
		EXPECT_EQ(range.front(), subobject);
		EXPECT_EQ(range.front()->guid(), poly.guid());
		EXPECT_TRUE(range.contains(subobject));
		EXPECT_FALSE(range.contains(reinterpret_cast<const HierarchyComponent*>(&poly)));
	}
}
