#include "pch.h"

#include <vector>
#include <Engine.ACS/Handle/HandleTable.hpp>

using namespace hg;
using namespace hg::engine::acs;

namespace Acs {
	TEST(Handle, ResolveAfterAcquire) {
		const auto space = acquireSpace();
		HandleTable table { space };
		int value = 0;

		const auto handle = table.acquire(&value);
		EXPECT_EQ(handle.space(), space);
		EXPECT_EQ(table.resolve(handle), &value);

		releaseSpace(space);
	}

	TEST(Handle, StaleAfterReleaseAndReuse) {
		HandleTable table { acquireSpace() };
		int a = 0, b = 0;

		const auto first = table.acquire(&a);
		table.release(first);
		EXPECT_EQ(table.resolve(first), nullptr);

		const auto second = table.acquire(&b);
		EXPECT_EQ(second.index(), first.index());
		EXPECT_NE(second.generation(), first.generation());
		EXPECT_EQ(table.resolve(first), nullptr);
		EXPECT_EQ(table.resolve(second), &b);

		releaseSpace(table.space());
	}

	TEST(Handle, Rebind) {
		HandleTable table { acquireSpace() };
		int a = 0, b = 0;

		const auto handle = table.acquire(&a);
		table.rebind(handle, &b);
		EXPECT_EQ(table.resolve(handle), &b);

		releaseSpace(table.space());
	}

	TEST(Handle, ReleaseAllInvalidatesSpace) {
		HandleTable table { acquireSpace() };
		int a = 0, b = 0;

		const auto ha = table.acquire(&a);
		const auto hb = table.acquire(&b);
		table.releaseAll();
		EXPECT_EQ(table.resolve(ha), nullptr);
		EXPECT_EQ(table.resolve(hb), nullptr);

		const auto hc = table.acquire(&a);
		EXPECT_EQ(table.resolve(hc), &a);
		EXPECT_EQ(table.resolve(ha), nullptr);

		releaseSpace(table.space());
	}

	TEST(Handle, PagedGrowth) {
		HandleTable table { acquireSpace() };
		std::vector<int> values(100000);
		std::vector<Handle> handles;
		handles.reserve(values.size());

		for (auto& value : values) {
			handles.push_back(table.acquire(&value));
		}
		for (size_t i = 0; i < values.size(); ++i) {
			ASSERT_EQ(table.resolve(handles[i]), &values[i]);
		}

		releaseSpace(table.space());
	}
}
