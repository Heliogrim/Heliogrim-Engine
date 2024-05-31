#pragma once

#include <concepts>

namespace hg {
	template <typename Ty_, typename... Types_>
	concept IsAnyOf = std::disjunction_v<std::is_same<Ty_, Types_>...>;
}
