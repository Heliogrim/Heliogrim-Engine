#include "pch.h"

#include <Engine.Filesystem/Path.hpp>
#include <Engine.Filesystem/__fwd.hpp>

using namespace hg;

namespace FileSystemModule {
	TEST(Path, IsLexicalSubPath) {
		const auto base = ::hg::fs::Path { "/x/dev/base/"sv }.normalized();
		const auto test = ::hg::fs::Path { "/x/dev/base/sub/val"sv }.normalized();

		EXPECT_FALSE(base.isSubPathOf(base));
		EXPECT_FALSE(base.isSubPathOf(test));
		EXPECT_TRUE(test.isSubPathOf(base));
	}

	TEST(Path, ContainsPathLexical) {
		const auto base = ::hg::fs::Path { "/x/dev/base/"sv }.normalized();
		const auto test = ::hg::fs::Path { "/x/dev/base/sub/val"sv }.normalized();

		EXPECT_FALSE(base.contains(base));
		EXPECT_TRUE(base.contains(test));
		EXPECT_FALSE(test.contains(base));
	}
}
