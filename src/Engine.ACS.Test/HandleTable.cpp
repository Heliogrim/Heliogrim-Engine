#include "pch.h"

#include <vector>
#include <Engine.ACS/Handle/HandleTable.hpp>

using namespace hg;
using namespace hg::engine::acs;

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
		Handle stale;
		{
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
		std::vector<int> values(100000);
		std::vector<Handle> handles;
		handles.reserve(values.size());

		for (auto& value : values) {
			handles.push_back(table.acquire(&value));
		}
		for (size_t i = 0; i < values.size(); ++i) {
			ASSERT_EQ(table.resolve(handles[i]), &values[i]);
		}
	}
}
