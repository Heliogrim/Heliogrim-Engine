#pragma once

#include <Engine.Common/__macro.hpp>

#include "../DataAttributes.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
	class TextureLikeObject;
}

namespace hg::engine::reflow {
	namespace attr {
		struct ImageData {
			constexpr static inline auto resource = "resource"sv;
			constexpr static inline auto image = "image"sv;
		};
	}

	struct ImageDataAttributes final :
		public DataAttributes<
			NamedAttribute<attr::ImageData::image, DataAttribute<nmpt<graphics::TextureLikeObject>>>,
			NamedAttribute<attr::ImageData::resource, DataAttribute<void*>>
		> {
		using DataAttributes::DataAttributes;
	};
}
