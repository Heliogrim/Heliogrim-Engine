#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg {
	namespace detail {
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

	constexpr inline detail::DiscardFn discard;
}
