#pragma once

#include <concepts>
#include <type_traits>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/Type.hpp>

namespace hg {
	template <typename Type_>
	struct clone_impl;

	/**/

	template <typename Type_>
	struct is_clonable {
		constexpr static auto value = std::is_copy_constructible_v<Type_>;
	};

	template <typename Type_, typename Dy_>
	struct is_clonable<UniquePtr<Type_, Dy_>> {
		constexpr static auto value = std::is_copy_constructible_v<Type_>;
	};

	template <typename Type_>
	concept Clonable = CompleteType<Type_> && is_clonable<Type_>::value;

	/**/

	template <Clonable Type_>
	[[nodiscard]] constexpr Type_ clone(const Type_& val_) {
		return clone_impl<Type_> {}(val_);
	}

	template <typename Type_> requires std::is_copy_constructible_v<Type_>
	struct clone_impl<uptr<Type_>> {
		[[nodiscard]] constexpr uptr<Type_> operator()(const uptr<Type_>& val_) const {
			return make_uptr<Type_>(static_cast<std::add_const_t<decltype(*val_)>>(*val_));
		}
	};

	template <typename Type_> requires std::is_copy_constructible_v<Type_>
	struct clone_impl<Type_> {
		[[nodiscard]] constexpr Type_ operator()(const Type_& val_) const noexcept {
			return Type_(val_);
		}
	};
}
