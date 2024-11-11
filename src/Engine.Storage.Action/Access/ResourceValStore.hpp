#pragma once

#include <concepts>
#include <type_traits>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/Address.hpp>
#include <Engine.Pedantic.Resource/AccessMode.hpp>

#include "IoResource.hpp"

namespace hg::engine::storage {
	/**/

	template <typename Type_>
	concept FancyPointerLike = requires(Type_&& obj_) {
		{ obj_.operator*() };
	};

	/**/

	template <typename ValueType_, typename MutableType_ = ValueType_, typename Type_ = std::add_const_t<MutableType_>>
	struct ResourceValStore {
		using ior_type = IoResource<MutableType_, Type_>;

		using mut_val_type = MutableType_;
		using mut_ret_type = std::add_lvalue_reference_t<MutableType_>;

		using const_val_type = Type_;
		using const_ret_type = std::add_lvalue_reference_t<Type_>;

		using value_type = ValueType_;
		mutable value_type value;

		[[nodiscard]] mut_ret_type resolveMutable(
			[[maybe_unused]] cref<ior_type> ior_,
			[[maybe_unused]] ResourceAccessMode mode_
		) const noexcept
			requires std::is_same_v<ValueType_, MutableType_> {
			return value;
		}

		[[nodiscard]] mut_ret_type resolveMutable(
			[[maybe_unused]] cref<ior_type> ior_,
			[[maybe_unused]] ResourceAccessMode mode_
		) const noexcept
			requires (not std::is_same_v<ValueType_, MutableType_>) &&
			FancyPointerLike<ValueType_> &&
			std::is_same_v<MutableType_, std::remove_const_t<std::remove_pointer_t<decltype(::hg::unfancy(value))>>> {
			return *hg::unfancy(value);
		}

		[[nodiscard]] const_ret_type resolveConstant(
			[[maybe_unused]] cref<ior_type> ior_,
			[[maybe_unused]] ResourceAccessMode mode_
		) const noexcept
			requires std::is_same_v<ValueType_, Type_> {
			return value;
		}

		[[nodiscard]] const_ret_type resolveConstant(
			[[maybe_unused]] cref<ior_type> ior_,
			[[maybe_unused]] ResourceAccessMode mode_
		) const noexcept
			requires (not std::is_same_v<ValueType_, Type_>) &&
			FancyPointerLike<ValueType_> &&
			(
				std::is_same_v<Type_, std::remove_const_t<std::remove_pointer_t<decltype(::hg::unfancy(value))>>> ||
				std::is_same_v<Type_, std::remove_pointer_t<decltype(::hg::unfancy(value))>>
			) {
			return *hg::unfancy(value);
		}
	};
}
