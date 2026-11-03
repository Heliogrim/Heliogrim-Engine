#include "pch.h"

/**/

#include <algorithm>
#include <Engine.ACS/Handle/HandleTable.hpp>
#include <Engine.Common/Collection/Vector.hpp>

using namespace hg::engine::acs;
using namespace hg;

namespace Acs {
	namespace {
		/** Releases the space on scope exit, also after a failed ASSERT. */
		struct SpaceGuard {
			SpaceId id = acquireSpace();

			~SpaceGuard() {
				releaseSpace(id);
			}
		};
	}

	TEST(Handle, ResolveAfterAcquire) {
		const SpaceGuard space {};
		HandleTable table { space.id };
		int value = 0;

		const auto handle = table.acquire(&value);
		EXPECT_EQ(handle.space(), space.id);
		EXPECT_EQ(table.resolve(handle), &value);
	}

	TEST(Handle, StaleAfterReleaseAndReuse) {
		const SpaceGuard space {};
		HandleTable table { space.id };
		int a = 0, b = 0;

		const auto first = table.acquire(&a);
		table.release(first);
		EXPECT_EQ(table.resolve(first), nullptr);

		const auto second = table.acquire(&b);
		EXPECT_EQ(second.index(), first.index());
		EXPECT_NE(second.generation(), first.generation());
		EXPECT_EQ(table.resolve(first), nullptr);
		EXPECT_EQ(table.resolve(second), &b);
	}

	TEST(Handle, GenerationWrapRetiresIndex) {
		// Fixed id outside the low range other tests acquire: the high-water mark of this id ends at the mask.
		HandleTable table { static_cast<SpaceId>(4095) };
		int value = 0;

		auto handle = table.acquire(&value);
		const auto index = handle.index();
		auto last = handle;
		for (u64 cycle = 0uLL; handle.index() == index && cycle <= Handle::generationMask + 1uLL; ++cycle) {
			last = handle;
			table.release(handle);
			handle = table.acquire(&value);
		}

		EXPECT_NE(handle.index(), index);
		EXPECT_EQ(last.generation(), Handle::generationMask);
		EXPECT_EQ(table.resolve(last), nullptr);
		EXPECT_EQ(table.resolve(handle), &value);

		// The saturated id is permanently exhausted: draining the registry never hands it out.
		Vector<SpaceId> drained {};
		for (auto id = acquireSpace(); id != detachedSpace; id = acquireSpace()) {
			drained.emplace_back(id);
		}
		for (const auto id : drained) {
			releaseSpace(id);
		}
		EXPECT_FALSE(drained.empty());
		EXPECT_EQ(std::ranges::find(drained, static_cast<SpaceId>(4095)), drained.end());
	}

	TEST(Handle, BigSpaceIdIsMasked) {
		HandleTable table { static_cast<SpaceId>(5000) };
		int value = 0;

		const auto handle = table.acquire(&value);
		EXPECT_EQ(table.space(), static_cast<SpaceId>(5000u & Handle::spaceMask));
		EXPECT_EQ(handle.space(), table.space());
		EXPECT_EQ(table.resolve(handle), &value);
	}

	TEST(Handle, Rebind) {
		const SpaceGuard space {};
		HandleTable table { space.id };
		int a = 0, b = 0;

		const auto handle = table.acquire(&a);
		table.rebind(handle, &b);
		EXPECT_EQ(table.resolve(handle), &b);
	}

	TEST(Handle, ReleaseAllInvalidatesSpace) {
		const SpaceGuard space {};
		HandleTable table { space.id };
		int a = 0, b = 0;

		const auto ha = table.acquire(&a);
		const auto hb = table.acquire(&b);
		table.releaseAll();
		EXPECT_EQ(table.resolve(ha), nullptr);
		EXPECT_EQ(table.resolve(hb), nullptr);

		// Reacquire twice so the indices of both old handles are live again.
		const auto hc = table.acquire(&a);
		const auto hd = table.acquire(&b);
		EXPECT_TRUE(hc.index() == ha.index() || hc.index() == hb.index());
		EXPECT_TRUE(hd.index() == ha.index() || hd.index() == hb.index());
		EXPECT_EQ(table.resolve(ha), nullptr);
		EXPECT_EQ(table.resolve(hb), nullptr);
		EXPECT_EQ(table.resolve(hc), &a);
		EXPECT_EQ(table.resolve(hd), &b);
	}

	TEST(Handle, StaleAfterSpaceIdReuse) {
		int a = 0, b = 0;

		const auto first = acquireSpace();
		Handle stale; {
			HandleTable table { first };
			stale = table.acquire(&a);
		}
		releaseSpace(first);

		const SpaceGuard second {};
		ASSERT_EQ(second.id, first);
		HandleTable table { second.id };
		const auto fresh = table.acquire(&b);
		EXPECT_EQ(fresh.index(), stale.index());
		EXPECT_EQ(table.resolve(stale), nullptr);
		EXPECT_EQ(table.resolve(fresh), &b);
	}

	TEST(Handle, PagedGrowth) {
		const SpaceGuard space {};
		HandleTable table { space.id };
		Vector<int> values(100000);
		Vector<Handle> handles;
		handles.reserve(values.size());

		for (auto& value : values) {
			handles.emplace_back(table.acquire(&value));
		}
		for (size_t i = 0; i < values.size(); ++i) {
			ASSERT_EQ(table.resolve(handles[i]), &values[i]);
		}
	}
}
