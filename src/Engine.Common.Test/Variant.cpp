#include "pch.h"
/**/

#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Variant.hpp>

using namespace hg;

namespace Common {
	TEST(Variant, Default) {
		auto obj = Variant<std::monostate> {};
		EXPECT_FALSE(obj.valueless_by_exception());
	}

	TEST(Variant, IsTypeCheck) {
		auto obj = Variant<u8, u16, u32, String, StringView> {};

		obj = u8 {};
		EXPECT_TRUE(obj.is<u8>());
		EXPECT_FALSE(obj.is<u16>());
		EXPECT_FALSE(obj.is<StringView>());

		obj = u32 {};
		EXPECT_FALSE(obj.is<u8>());
		EXPECT_TRUE(obj.is<u32>());
		EXPECT_FALSE(obj.is<StringView>());

		obj = "View"sv;
		EXPECT_FALSE(obj.is<String>());
		EXPECT_TRUE(obj.is<StringView>());
	}

	TEST(Variant, AsType) {
		auto obj = Variant<u32, StringView> {};

		obj = "Test View"sv;
		EXPECT_EQ(obj.as<StringView>(), "Test View"sv);

		obj = u32 { 675714 };
		EXPECT_EQ(obj.as<u32>(), 675714);
	}

	TEST(Variant, DefaultHoldsFirstAlternative) {
		auto obj = Variant<u32, StringView> {};

		EXPECT_TRUE(obj.is<u32>());
		EXPECT_FALSE(obj.is<StringView>());
		EXPECT_EQ(obj.as<u32>(), 0u);
	}

	TEST(Variant, ReassignSwitchesAlternativeBothWays) {
		auto obj = Variant<u32, String> {};

		obj = String { "a string beyond the small buffer optimization" };
		EXPECT_TRUE(obj.is<String>());
		EXPECT_FALSE(obj.is<u32>());

		obj = u32 { 9 };
		EXPECT_TRUE(obj.is<u32>());
		EXPECT_FALSE(obj.is<String>());
		EXPECT_EQ(obj.as<u32>(), 9u);
	}

	TEST(Variant, CopyKeepsAlternativeAndValue) {
		const auto src = Variant<u32, String> { String { "a string beyond the small buffer optimization" } };
		auto dst = src;

		ASSERT_TRUE(dst.is<String>());
		EXPECT_EQ(dst.as<String>(), "a string beyond the small buffer optimization");
		EXPECT_EQ(src.as<String>(), dst.as<String>());
	}

	TEST(Variant, MoveTransfersValue) {
		auto src = Variant<u32, String> { String { "a string beyond the small buffer optimization" } };
		auto dst = ::hg::move(src);

		ASSERT_TRUE(dst.is<String>());
		EXPECT_EQ(dst.as<String>(), "a string beyond the small buffer optimization");
	}

	TEST(Variant, ApplyVisitsActiveAlternativeOnly) {
		auto obj = Variant<u32, StringView> { "view"sv };

		const auto visited = obj.apply(
			[]<typename Ty_>(const Ty_&) { return std::is_same_v<Ty_, StringView>; }
		);
		EXPECT_TRUE(visited);
	}

	struct VariantTestBase {
		[[nodiscard]] int val() const noexcept {
			return 45;
		}

		[[nodiscard]] int val() noexcept {
			return 86;
		}
	};

	struct VariantTestDerived1 : public VariantTestBase {};

	struct VariantTestDerived2 : public VariantTestBase {};

	struct VariantTestDerivedLayer : public VariantTestDerived2 {};

	TEST(Variant, CommonTypeInvocation) {
		using type = Variant<VariantTestBase, VariantTestDerived1, VariantTestDerived2, VariantTestDerivedLayer>;

		auto obj = type {};
		EXPECT_EQ(obj->val(), 86);

		const auto cobj = type {};
		EXPECT_EQ(cobj->val(), 45);
	}

	TEST(Variant, InvokeFromShared) {
		auto obj = Variant<VariantTestDerived1, VariantTestDerived2, VariantTestDerivedLayer> {};
		EXPECT_EQ(obj.apply([](auto&& stored_) { return std::forward<decltype(stored_)>(stored_).val(); }), 86);

		const auto cobj = Variant<VariantTestDerived1, VariantTestDerived2, VariantTestDerivedLayer> {};
		EXPECT_EQ(cobj.apply([](const auto& stored_) { return stored_.val(); }), 45);
	}
}
