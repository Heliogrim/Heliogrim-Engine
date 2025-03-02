#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../DataAttributes.hpp"

namespace hg::engine::gfx {
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
			NamedAttribute<attr::ImageData::image, DataAttribute<nmpt<gfx::TextureLikeObject>>>,
			NamedAttribute<attr::ImageData::resource, DataAttribute<void*>>
		> {
		using DataAttributes::DataAttributes;
	};
}
