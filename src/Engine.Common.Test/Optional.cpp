#include "pch.h"

#include <string>
#include <Engine.Common/Optional.hpp>

using namespace hg;

namespace Common {
	namespace {
		struct Probe {
			int a;
			int b;

			constexpr Probe(int a_, int b_) noexcept :
				a(a_),
				b(b_) {}
		};
	}

	TEST(OptionalSome, lvalueBindsReference) {
		int value = 5;
		auto opt = Some(value);

		static_assert(std::is_same_v<decltype(opt), Opt<int&>>);
		EXPECT_FALSE(opt.is_null());
		EXPECT_EQ(std::addressof(opt.value()), std::addressof(value));

		value = 9;
		EXPECT_EQ(opt.value(), 9);
	}

	TEST(OptionalSome, constLvalueBindsConstReference) {
		const int value = 5;
		auto opt = Some(value);

		static_assert(std::is_same_v<decltype(opt), Opt<const int&>>);
		EXPECT_EQ(std::addressof(opt.value()), std::addressof(value));
	}

	TEST(OptionalSome, explicitRefType) {
		int value = 5;
		auto opt = Some<int>(value);

		static_assert(std::is_same_v<decltype(opt), Opt<int&>>);
		EXPECT_EQ(std::addressof(opt.value()), std::addressof(value));
	}

	TEST(OptionalSome, rvalueMovesIntoValue) {
		auto opt = Some(std::string { "a string beyond the small buffer optimization" });

		static_assert(std::is_same_v<decltype(opt), Opt<std::string>>);
		EXPECT_EQ(opt.value(), "a string beyond the small buffer optimization");
	}

	TEST(OptionalSome, movedFromSourceIsConsumed) {
		std::string src { "a string beyond the small buffer optimization" };
		auto opt = Some(::hg::move(src));

		EXPECT_EQ(opt.value(), "a string beyond the small buffer optimization");
		EXPECT_TRUE(src.empty());
	}

	TEST(OptionalSomeRef, bindsLvalue) {
		int value = 1;
		auto opt = SomeRef(value);

		static_assert(std::is_same_v<decltype(opt), Opt<int&>>);
		opt.value() = 2;
		EXPECT_EQ(value, 2);
	}

	TEST(OptionalSomeRef, bindsConstLvalue) {
		const int value = 1;
		auto opt = SomeRef(value);

		static_assert(std::is_same_v<decltype(opt), Opt<const int&>>);
		EXPECT_EQ(std::addressof(opt.value()), std::addressof(value));
	}

	TEST(OptionalSomeVal, copiesLvalue) {
		int value = 1;
		auto opt = SomeVal(value);

		static_assert(std::is_same_v<decltype(opt), Opt<int>>);
		value = 2;
		EXPECT_EQ(opt.value(), 1);
	}

	TEST(OptionalSomeVal, movesRvalue) {
		std::string src { "a string beyond the small buffer optimization" };
		auto opt = SomeVal(::hg::move(src));

		static_assert(std::is_same_v<decltype(opt), Opt<std::string>>);
		EXPECT_EQ(opt.value(), "a string beyond the small buffer optimization");
	}

	TEST(OptionalSomeVal, constructsInPlaceFromArgs) {
		auto opt = SomeVal<Probe>(3, 4);

		static_assert(std::is_same_v<decltype(opt), Opt<Probe>>);
		EXPECT_EQ(opt.value().a, 3);
		EXPECT_EQ(opt.value().b, 4);
	}

	TEST(OptionalSomeVal, explicitTypeSingleArg) {
		auto opt = SomeVal<Probe>(Probe { 5, 6 });

		static_assert(std::is_same_v<decltype(opt), Opt<Probe>>);
		EXPECT_EQ(opt.value().a, 5);
	}

	TEST(OptionalNone, isNull) {
		Opt<int&> opt = None;
		EXPECT_TRUE(opt.is_null());
		EXPECT_EQ(opt.address(), nullptr);
	}
}
