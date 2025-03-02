#pragma once

#include <tuple>

#include "NamedAttribute.hpp"

namespace hg::engine::reflow {
	template <typename... NamedAttributes_>
	struct named_attribute_tuple {
		using type = std::tuple<NamedAttributes_...>;
	};
}
