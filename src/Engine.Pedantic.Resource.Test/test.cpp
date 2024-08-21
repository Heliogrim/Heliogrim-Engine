#include "pch.h"

#include <Engine.Pedantic.Resource/ExclusiveResource.hpp>

using namespace hg;

namespace PedanticResourceModule {
	TEST(ExclusiveResource, ChangeResourceValue) {

		constexpr auto initial_value = 23588269688uLL;
		constexpr auto override_value = 658765825uLL;

		auto obj = hg::make_exclusive_resource<u64>(initial_value);
		obj.apply(
			[](ref<u64> val_) {
				val_ = override_value;
			}
		);

		EXPECT_EQ(obj.acquireReadonly().get(), override_value);
	}

	TEST(ExclusiveResource, ReportResourceValue) {

		constexpr auto initial_value = 431567930165uLL;
		constexpr auto validation_value = initial_value * 2uLL;

		auto obj = make_exclusive_resource<const u64>(initial_value);
		const auto result = obj.apply(
			[](cref<u64> val_) {
				return val_ * 2uLL;
			}
		);

		EXPECT_EQ(result, validation_value);
		EXPECT_EQ(obj.acquireReadonly().get(), initial_value);

	}

	TEST(ExclusiveResource, MoveTrivialResource) {

		constexpr auto initial_value = 7258678751uLL;

		auto initial = make_exclusive_resource<u64>(initial_value);
		auto target = ExclusiveResource<u64> { std::move(initial) };

		EXPECT_EQ(target.acquireReadonly().get(), initial_value);

	}

	TEST(ExclusiveResource, RefCountMoveResource) {

		using type = std::shared_ptr<u64>;

		constexpr auto initial_value = 652897529uLL;
		auto payload = std::make_shared<u64>(initial_value);

		auto initial = make_exclusive_resource<decltype(payload)>(payload);
		EXPECT_EQ(payload.use_count(), 2uLL);

		auto target = ExclusiveResource<decltype(payload)> { std::move(initial) };
		EXPECT_EQ(payload.use_count(), 2uLL);

		EXPECT_EQ(*target.acquireReadonly().get(), initial_value);

		payload.reset();
		EXPECT_EQ(target.acquireReadonly().get().use_count(), 1uLL);
	}

	TEST(ExclusiveResource, MoveMoveOnlyResource) {

		using type = std::unique_ptr<u64>;

		constexpr auto initial_value = 9235768875uLL;
		auto payload = std::make_unique<u64>(initial_value);

		auto initial = make_exclusive_resource<decltype(payload)>(std::move(payload));
		EXPECT_EQ(payload.get(), nullptr);
		EXPECT_NE(initial.acquireReadonly().get().get(), nullptr);

		auto target = ExclusiveResource<decltype(payload)> { std::move(initial) };
		EXPECT_EQ(initial.acquireReadonly().get().get(), nullptr);
		EXPECT_NE(target.acquireReadonly().get().get(), nullptr);
	}

	class MoveConstructOnly {
	public:
		u64 payload = 0uLL;

	public:
		constexpr MoveConstructOnly() noexcept = default;

		constexpr MoveConstructOnly(const decltype(payload) val_) noexcept :
			payload(val_) {}

		MoveConstructOnly(const MoveConstructOnly&) = delete;

		MoveConstructOnly(MoveConstructOnly&&) noexcept = default;

		constexpr ~MoveConstructOnly() noexcept = default;

		MoveConstructOnly& operator=(const MoveConstructOnly&) = delete;

		MoveConstructOnly& operator=(MoveConstructOnly&&) = delete;
	};

	TEST(ExclusiveResource, MoveMoveConstructOnlyResource) {

		constexpr auto initial_value = 47562467uLL;

		auto initial = make_exclusive_resource<MoveConstructOnly>(initial_value);
		EXPECT_EQ(initial.acquireReadonly().get().payload, initial_value);

		auto target = ExclusiveResource<MoveConstructOnly> { std::move(initial) };
		EXPECT_EQ(target.acquireReadonly().get().payload, initial_value);

	}

	TEST(ExclusiveResource, PreventSecondAccess) {

		auto initial = make_exclusive_resource<u64>(); {
			const auto first_access = initial.tryAcquireReadonly();
			EXPECT_TRUE(first_access.has_value());

			const auto second_access = initial.tryAcquireReadonly();
			EXPECT_FALSE(second_access.has_value());
		} {
			const auto third_access = initial.tryAcquireReadonly();
			EXPECT_TRUE(third_access.has_value());
		}
	}

	TEST(ExclusiveResource, TryChangeResourceValue) {

		constexpr auto initial_value = 847358826uLL;
		constexpr auto override_value = 346878688uLL;

		const auto initial = make_exclusive_resource<u64>(initial_value);
		EXPECT_EQ(initial.acquireReadonly().get(), initial_value);

		const auto result = initial.tryApply(
			[](ref<u64> val_) {
				val_ = override_value;
			}
		);

		EXPECT_TRUE(result);
		EXPECT_EQ(initial.acquireReadonly().get(), override_value);
	}

	TEST(ExclusiveResource, PreventSecondApply) {

		auto initial = make_exclusive_resource<u64>();

		const auto exclusive_access = initial.acquireReadonly();

		const auto first_result = initial.tryApply(
			[](ref<u64> val_) {
				val_ = 6569878286uLL;
			}
		);
		EXPECT_FALSE(first_result);

		const auto second_result = initial.tryApply(
			[](cref<u64> val_) {
				return val_ * 367uLL;
			}
		);
		EXPECT_FALSE(second_result.first);
		EXPECT_FALSE(second_result.second.has_value());
	}
}
