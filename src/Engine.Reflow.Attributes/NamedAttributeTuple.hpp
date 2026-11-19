#pragma once

#include "NamedAttribute.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg::engine::reflow {
	template <typename... NamedAttributes_>
	struct named_attribute_tuple {
		using type = std::tuple<NamedAttributes_...>;
	};
}
