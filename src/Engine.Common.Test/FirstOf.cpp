#include "pch.h"

#include <vector>
#include <Engine.Common/Algo/FirstOf.hpp>
#include <Engine.Common/Functional/FnRef.hpp>
#include <Engine.Common/String/Convert.hpp>

using namespace hg;

namespace Common {
	TEST(FirstOf, ReturnsFirstMatch) {
		const std::vector<int> values { 1, 2, 3, 4 };

		const auto found = FirstOf<Opt<int>>(
			values,
			[](const int v_) -> Opt<int> { return v_ > 2 ? Opt<int> { SomeVal(v_ * 10) } : Opt<int> { None }; }
		);

		ASSERT_TRUE(found.has_value());
		EXPECT_EQ(found.value(), 30);
	}

	TEST(FirstOf, ReturnsNoneWhenNoMatch) {
		const std::vector<int> values { 1, 2 };

		const auto found = FirstOf<Opt<int>>(values, [](const int) -> Opt<int> { return None; });

		EXPECT_FALSE(found.has_value());
	}

	TEST(FnRef, InvokesCallable) {
		int calls = 0;
		auto fn = [&calls](const int v_) { calls += v_; return calls; };

		FnRef<int(int)> ref { fn };
		EXPECT_EQ(ref(2), 2);
		EXPECT_EQ(ref(3), 5);
		EXPECT_EQ(calls, 5);
	}

	#ifdef WIN32
	TEST(StringConvert, AsciiToWide) {
		EXPECT_EQ(stringToWString("heliogrim"), L"heliogrim");
		EXPECT_EQ(stringToWString(""), L"");
	}
	#endif
}
