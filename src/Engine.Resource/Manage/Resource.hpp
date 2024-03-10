#pragma once

#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

#include "ResourceBase.hpp"
#include "TypedManageGuard.hpp"
#include "../ResourceUsageFlag.hpp"

namespace hg::engine::resource {
	template <typename ManagedType_>
	class __declspec(novtable) Resource :
		public ResourceBase {
	public:
		using this_type = Resource<ManagedType_>;

		using value_type = ManagedType_;
		using loaded_flag_type = u8;

	protected:
		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 27.08.2021
		 */
		constexpr Resource() noexcept:
			ResourceBase(),
			_value(nullptr) {}

		template <typename Type_ = ManagedType_> requires std::is_default_constructible_v<Type_>
		constexpr Resource(std::in_place_t) noexcept(std::is_nothrow_default_constructible_v<Type_>) :
			ResourceBase(),
			_value(make_ptr<value_type>()) {}

		constexpr Resource(value_type* __restrict value_) noexcept :
			ResourceBase(),
			_value(value_) {}

		template <typename... Args_> requires std::is_constructible_v<value_type, Args_...>
		constexpr Resource(Args_&&... args_) noexcept(std::is_nothrow_constructible_v<value_type, Args_...>) :
			ResourceBase(),
			_value(make_ptr<value_type, Args_...>(std::forward<Args_>(args_)...)) {}

	public:
		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 27.08.2021
		 */
		~Resource() override = default;

	private:
		__restricted_ptr<value_type> _value;

	public:
		loaded_flag_type loaded() const noexcept override {
			return (loaded_flag_type)(_value != nullptr);
		}

	protected:
		[[nodiscard]] const non_owning_rptr<void> value() const noexcept override {
			return _value;
		}

	public:
		/**
		 * Acquire this resource to use
		 *
		 * @author Julius
		 * @date 28.08.2021
		 *
		 * @param  flags_ (Optional) The flags.
		 *
		 * @returns A TypedManageGuard instance.
		 */
		template <typename Type_ = ManagedType_> requires std::is_same_v<Type_, ManagedType_>
		[[nodiscard]] TypedManageGuard<Type_> acquire(
			const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
		) {
			auto guard = static_cast<ptr<ResourceBase>>(this)->acquire(flags_);
			return static_cast<mref<TypedManageGuard<Type_>>>(guard);
		}

		/**
		 * Attempts to acquire this resource to use
		 *
		 * @author Julius
		 * @date 28.08.2021
		 *
		 * @param  guard_ The guard.
		 * @param  flags_ (Optional) The flags.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		template <typename Type_ = ManagedType_> requires std::is_same_v<Type_, ManagedType_>
		[[nodiscard]] bool try_acquire(
			_Out_ ref<TypedManageGuard<Type_>> guard_,
			const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
		) noexcept {
			return static_cast<ptr<ResourceBase>>(this)->try_acquire(static_cast<ref<ManageGuard>>(guard_), flags_);
		}
	};
}
