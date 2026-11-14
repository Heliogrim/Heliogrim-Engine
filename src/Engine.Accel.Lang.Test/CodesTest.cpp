#include "pch.h"

/**/

#include <Engine.Accel.Lang/Diagnostic/Codes.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <algorithm>

using namespace hg::engine::accel::lang;
using namespace hg;

namespace Codes {
	TEST(Codes, TableIsWellFormed) {
		Vector<StringView> seen {};
		u16 expected = 1u;
		for (const auto& info : codes::table) {
			EXPECT_EQ(StringView(info.code.domain.data()), "ASL");
			EXPECT_EQ(info.code.number, expected) << info.key;
			EXPECT_FALSE(info.summary.empty()) << info.key;

			const auto& key = info.key;
			EXPECT_FALSE(key.empty());
			EXPECT_TRUE(std::ranges::all_of(key, [](char c_) { return (c_ >= 'a' && c_ <= 'z') || (c_ >= '0' && c_ <= '9') || c_ == '-'; })) << key;
			EXPECT_FALSE(key.starts_with('-') || key.ends_with('-') || key.contains("--")) << key;
			EXPECT_FALSE(std::ranges::contains(seen, key)) << key;

			// internal-* codes are compiler bugs, never the shader author's fault
			EXPECT_EQ(key.starts_with("internal-"), info.summary.starts_with("compiler bug: ")) << key;

			seen.emplace_back(key);
			++expected;
		}
	}
}
