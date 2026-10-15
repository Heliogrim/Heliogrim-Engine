#include "pch.h"

#include <string>
#include <Engine.Common/Expected.hpp>

using namespace hg;

namespace Common {
	TEST(Result, ExpectedConstructsValue) {
		Result<int, std::string> res = Expected<int> { 5 };

		ASSERT_TRUE(res.has_value());
		EXPECT_EQ(res.value(), 5);
	}

	TEST(Result, UnexpectedConstructsError) {
		Result<int, std::string> res = Unexpected<std::string> { "failed" };

		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error(), "failed");
	}

	TEST(Result, VoidExpected) {
		Result<void, int> res = Expected<void> {};
		EXPECT_TRUE(res.has_value());
	}

	TEST(Result, StaticFactories) {
		const auto ok = Result<int, int>::expected(3);
		const auto err = Result<int, int>::unexpected(9);

		EXPECT_TRUE(ok.has_value());
		EXPECT_EQ(ok.value(), 3);
		EXPECT_FALSE(err.has_value());
		EXPECT_EQ(err.error(), 9);
	}

	TEST(Result, ValueOrFallsBackOnError) {
		const Result<int, int> err = Unexpected<int> { 1 };
		EXPECT_EQ(err.value_or(42), 42);
	}
}
