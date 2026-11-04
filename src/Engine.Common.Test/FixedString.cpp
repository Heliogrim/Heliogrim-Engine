#include "pch.h"

#include <type_traits>
#include <Engine.Common/Meta/FixedString.hpp>

using namespace hg;

namespace Common {
	template <fixed_string S>
	constexpr auto name_of() {
		return S.view();
	}

	template <fixed_string S>
	struct Tagged {};

	TEST(FixedString, NttpEquality) {
		static_assert(name_of<"wheels">() == "wheels");
		EXPECT_EQ(name_of<"wheels">(), "wheels");
		EXPECT_EQ(fixed_string { "abc" }.size(), 3u);

		static_assert(std::is_same_v<Tagged<"a">, Tagged<"a">>);
		static_assert(not std::is_same_v<Tagged<"a">, Tagged<"b">>);
		EXPECT_TRUE(fixed_string { "x" } == fixed_string { "x" });
		EXPECT_FALSE(fixed_string { "x" } == fixed_string { "y" });
	}
}
