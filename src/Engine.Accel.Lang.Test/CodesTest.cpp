#include "pch.h"

/**/

#include <Engine.Accel.Lang/Diagnostic/Codes.hpp>

import Engine.Common;

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
			EXPECT_TRUE(
				std::ranges::all_of(key, [](char c_) { return (c_ >= 'a' && c_ <= 'z') || (c_ >= '0' && c_ <= '9') || c_ == '-'; })
			) << key;
			EXPECT_FALSE(key.starts_with('-') || key.ends_with('-') || key.contains("--")) << key;
			EXPECT_FALSE(std::ranges::contains(seen, key)) << key;

			// internal-* codes are compiler bugs, never the shader author's fault
			EXPECT_EQ(key.starts_with("internal-"), info.summary.starts_with("compiler bug: ")) << key;

			seen.emplace_back(key);
			++expected;
		}
	}

	TEST(Codes, SummariesFollowWording) {
		for (const auto& info : codes::table) {
			const StringView summary = info.summary;
			EXPECT_FALSE(summary.empty()) << info.key;
			if (summary.empty()) {
				continue;
			}

			EXPECT_FALSE(summary.front() >= 'A' && summary.front() <= 'Z') << info.key << ": " << summary;
			EXPECT_NE(summary.back(), '.') << info.key << ": " << summary;

			// compiler jargon the wording rules forbid: prefix match for stems, whole word for short ones
			for (const StringView stem : { StringView { "lane" }, StringView { "wave" }, StringView { "derivat" } }) {
				EXPECT_EQ(summary.find(stem), StringView::npos) << info.key << ": '" << stem << "' in " << summary;
			}
			const auto isWord = [](char c_) { return (c_ >= 'a' && c_ <= 'z') || (c_ >= 'A' && c_ <= 'Z') || (c_ >= '0' && c_ <= '9'); };
			for (const StringView banned : { StringView { "ir" }, StringView { "lowering" } }) {
				for (auto at = summary.find(banned); at != StringView::npos; at = summary.find(banned, at + 1uz)) {
					const bool before = at > 0uz && isWord(summary[at - 1uz]);
					const bool after = at + banned.size() < summary.size() && isWord(summary[at + banned.size()]);
					EXPECT_TRUE(before || after) << info.key << ": '" << banned << "' in " << summary;
				}
			}
		}
	}
}
