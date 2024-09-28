#pragma once

#include <concepts>

namespace hg {
	template <class Left_, class Right_>
	concept same_as = std::same_as<Left_, Right_>;

	template <class Left_, class Right_>
	concept not_same_as = not same_as<Left_, Right_>;
}
