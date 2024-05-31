#include "pch.h"

#include <Engine.Common/Make.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Config/Provider/RuntimeProvider.hpp>
#include <Engine.Config/Provider/SystemProvider.hpp>

using namespace hg;

namespace ConfigModule {
	TEST(Config, BaseLifecycle) {
		auto obj = make_uptr<engine::Config>();
		obj.reset();
	}

	TEST(Config, ProviderLifecycle) {
		auto obj = make_uptr<engine::Config>();

		auto prov = make_uptr<engine::cfg::SystemProvider>();
		const auto& stored = obj->registerProvider(std::move(prov));
		auto ret = obj->unregisterProvider(stored);

		obj.reset();
	}

	TEST(Config, InitWithEnumKey) {
		auto obj = make_uptr<engine::Config>();
		const auto& prov = obj->registerProvider(make_uptr<engine::cfg::SystemProvider>());

		/**/

		{
			auto result = obj->init(
				engine::cfg::ProjectConfigProperty::eProjectTitle,
				"Hello World"sv,
				query_provider_id(prov)
			);

			ASSERT_TRUE(result.has_value());
		}

		/**/

		{
			auto result = obj->get(engine::cfg::ProjectConfigProperty::eProjectTitle);

			ASSERT_TRUE(result.has_value());
			ASSERT_TRUE(std::holds_alternative<StringView>(result.value()));
			ASSERT_EQ(std::get<StringView>(result.value()), "Hello World"sv);
		}

		/**/

		obj.reset();
	}

	TEST(Config, InitWithStringKey) {
		auto obj = make_uptr<engine::Config>();
		const auto& prov = obj->registerProvider(make_uptr<engine::cfg::SystemProvider>());

		/**/

		{
			auto result = obj->init(
				"WildcardKey"sv,
				"Hello World"sv,
				query_provider_id(prov)
			);

			ASSERT_TRUE(result.has_value());
		}

		/**/

		{
			auto result = obj->get("WildcardKey"sv);

			ASSERT_TRUE(result.has_value());
			ASSERT_TRUE(std::holds_alternative<StringView>(result.value()));
			ASSERT_EQ(std::get<StringView>(result.value()), "Hello World"sv);
		}

		/**/

		obj.reset();
	}

	template <typename ValueType_>
	constexpr void test_config_value_type(ValueType_&& value_, StringView key_ = "TypeCheckKey"sv) {
		auto obj = make_uptr<engine::Config>();
		const auto& prov = obj->registerProvider(make_uptr<engine::cfg::SystemProvider>());

		using OptValueType = std::remove_cvref_t<ValueType_>;
		const auto copy = OptValueType { static_cast<cref<OptValueType>>(value_) };

		/**/

		{
			auto result = obj->init(
				key_,
				std::forward<ValueType_>(value_),
				query_provider_id(prov)
			);
			ASSERT_TRUE(result.has_value());
		}

		/**/

		{
			auto result = obj->getTyped<OptValueType>(key_);
			ASSERT_TRUE(result.has_value());
			ASSERT_EQ(result.value(), copy);
		}

		/**/

		obj.reset();
	}

	TEST(Config, ValueTypeMonostate) {
		test_config_value_type(std::monostate {});
	}

	TEST(Config, ValueTypeBool) {
		test_config_value_type(true);
	}

	TEST(Config, ValueTypeU32) {
		test_config_value_type(u32 { 123456 });
	}

	TEST(Config, ValueTypeF32) {
		test_config_value_type(f32 { 3.141F });
	}

	TEST(Config, ValueTypeStringView) {
		test_config_value_type("StringView Payload"sv);
	}

	TEST(Config, ValueTypeString) {
		test_config_value_type(String { "String Payload" });
	}

	TEST(Config, UpdateValue) {
		auto obj = make_uptr<engine::Config>();
		const auto& prov = obj->registerProvider(make_uptr<engine::cfg::SystemProvider>());
		constexpr auto key = engine::cfg::ProjectConfigProperty::eProjectTitle;

		/**/

		{
			const auto result = obj->init(key, "Test Title"sv, engine::cfg::query_provider_id(prov));
			ASSERT_TRUE(result.has_value());
			ASSERT_EQ(obj->getTyped<StringView>(key).value(), "Test Title"sv);
		}

		/**/

		{
			const auto result = obj->update(key, "Updated Title"sv);
			ASSERT_TRUE(result.has_value());
			ASSERT_EQ(obj->getTyped<StringView>(key).value(), "Updated Title"sv);
		}

		/**/

		obj.reset();
	}

	TEST(Config, UpdateWrongKey) {
		auto obj = make_uptr<engine::Config>();
		const auto& prov = obj->registerProvider(make_uptr<engine::cfg::SystemProvider>());

		constexpr auto key = engine::cfg::ProjectConfigProperty::eProjectTitle;
		constexpr auto failKey = engine::cfg::ProjectConfigProperty::eLocalBasePath;

		/**/

		const auto init = obj->init(key, "Test Title"sv, engine::cfg::query_provider_id(prov));
		const auto update = obj->update(failKey, "Updated Title"sv);

		EXPECT_FALSE(update.has_value());

		/**/

		obj.reset();
	}

	TEST(Config, UpdateWrongType) {
		auto obj = make_uptr<engine::Config>();
		const auto& prov = obj->registerProvider(make_uptr<engine::cfg::SystemProvider>());
		constexpr auto key = engine::cfg::ProjectConfigProperty::eProjectTitle;

		/**/

		const auto init = obj->init(key, "Test Title"sv, engine::cfg::query_provider_id(prov));
		const auto update = obj->update(key, std::monostate {});

		EXPECT_FALSE(update.has_value());

		/**/

		obj.reset();
	}

	TEST(Config, Layering) {
		auto obj = make_uptr<engine::Config>();

		const auto& sys = obj->registerProvider(make_uptr<engine::cfg::SystemProvider>());
		const auto& run = obj->registerProvider(make_uptr<engine::cfg::RuntimeProvider>());

		constexpr auto sysKey = "System Key"sv;
		constexpr auto runKey = "Runtime Key"sv;
		constexpr auto testKey = "Test Key"sv;

		/**/

		ASSERT_TRUE(obj->init(sysKey, "System Value"sv, engine::cfg::query_provider_id(sys)).has_value());
		ASSERT_TRUE(obj->init(runKey, "Runtime Value"sv, engine::cfg::query_provider_id(run)).has_value());

		ASSERT_TRUE(obj->init(testKey, "Test Value on System"sv, engine::cfg::query_provider_id(sys)).has_value());
		ASSERT_TRUE(obj->init(testKey, "Test Value on Dynamic"sv).has_value());

		/**/

		ASSERT_EQ(obj->getTyped<StringView>(sysKey), "System Value"sv);
		ASSERT_EQ(obj->getTyped<StringView>(runKey), "Runtime Value"sv);

		ASSERT_EQ(obj->getTyped<StringView>(testKey), "Test Value on Dynamic"sv);

		/**/

		obj->unregisterProvider(run);

		/**/

		ASSERT_EQ(obj->getTyped<StringView>(sysKey), "System Value"sv);
		ASSERT_FALSE(obj->get(runKey).has_value());
		ASSERT_EQ(obj->getTyped<StringView>(testKey), "Test Value on System"sv);

		/**/

		obj.reset();
	}
}
