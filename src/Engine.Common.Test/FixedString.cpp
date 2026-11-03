#include "pch.h"

#include <type_traits>
#include <Engine.Common/Meta/FixedString.hpp>

using namespace hg;

namespace Common {
	template <FixedString S>
	constexpr auto name_of() {
		return S.view();
	}

	template <FixedString S>
	struct Tagged {};

	TEST(FixedString, NttpEquality) {
		static_assert(name_of<"wheels">() == "wheels");
		EXPECT_EQ(name_of<"wheels">(), "wheels");
		EXPECT_EQ(FixedString { "abc" }.size(), 3u);

		static_assert(std::is_same_v<Tagged<"a">, Tagged<"a">>);
		static_assert(not std::is_same_v<Tagged<"a">, Tagged<"b">>);
		EXPECT_TRUE(FixedString { "x" } == FixedString { "x" });
		EXPECT_FALSE(FixedString { "x" } == FixedString { "y" });
	}
}
