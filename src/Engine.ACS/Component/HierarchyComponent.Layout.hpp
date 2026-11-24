#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "HierarchyComponent.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::HierarchyComponent> {
	public:
		// Transform and bounds are not part of the layout stream; a plain hierarchy slot streams nothing.
		inline constexpr static std::array<::hg::engine::serialization::access::LayoutField, 0uLL> fields {};
	};
}
