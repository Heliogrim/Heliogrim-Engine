#pragma once

#include <tuple>

namespace hg::engine::reflow {
	template <class... AttributeSets_>
	struct CompositeLayoutAttributes {
	public:
		using this_type = CompositeLayoutAttributes<AttributeSets_...>;
		using attribute_set_tuple = std::tuple<AttributeSets_...>;

	public:
		attribute_set_tuple attributeSets;
	};
}
