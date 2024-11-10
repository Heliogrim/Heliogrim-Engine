#pragma once

#include <type_traits>

namespace hg {
	namespace {
		struct DiscardFn {
			template <typename Type_> requires std::is_move_constructible_v<Type_> && std::is_destructible_v<Type_>
			[[maybe_unused]] constexpr const DiscardFn& operator=([[maybe_unused]] Type_&& obj_) const noexcept {
				[[maybe_unused]] Type_ disposed { std::forward<Type_>(obj_) };
				return *this;
			}

			template <typename Type_> requires std::is_move_constructible_v<Type_> && std::is_destructible_v<Type_>
			constexpr void operator()([[maybe_unused]] Type_&& obj_) const noexcept {
				[[maybe_unused]] Type_ disposed { std::forward<Type_>(obj_) };
			}
		};
	}

	constexpr static DiscardFn discard;
}
