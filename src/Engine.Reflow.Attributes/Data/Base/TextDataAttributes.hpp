#pragma once

#include "../DataAttributes.hpp"

namespace hg::engine::reflow {
	namespace attr {
		struct TextData {
			constexpr static inline auto text = "text"sv;
		};
	}

	struct TextDataAttributes final :
		public DataAttributes<
			NamedAttribute<attr::TextData::text, DataAttribute<String>>
		> {
		using DataAttributes::DataAttributes;
	};
}
