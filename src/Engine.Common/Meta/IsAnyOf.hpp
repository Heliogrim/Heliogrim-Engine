#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg {
	template <typename Ty_, typename... Types_>
	concept IsAnyOf = std::disjunction_v<std::is_same<Ty_, Types_>...>;
}
