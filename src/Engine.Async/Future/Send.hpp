#pragma once

#include <type_traits>
#include <Engine.Common/Wrapper.hpp>

namespace hg {
	// Types opt out by declaring a public `not_send` alias; a raw ptr<void> carries no ownership information.
	template <typename T_>
	inline constexpr bool is_send_v = not requires { typename T_::not_send; } && not std::is_same_v<std::remove_cv_t<T_>, ptr<void>>;

	template <>
	inline constexpr bool is_send_v<void> = true;

	template <typename T_>
	concept Send = is_send_v<T_>;
}
