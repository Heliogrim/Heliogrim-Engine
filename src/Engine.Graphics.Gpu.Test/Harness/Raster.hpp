#pragma once

#include <Engine.Common/__macro.hpp>
#include <cassert>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::test::gpu {
	struct Raster {
		math::uivec2 extent;
		Vector<math::fvec4> texels;

		/**
		 * @details Row 0 is the top row.
		 */
		[[nodiscard]] ref<const math::fvec4> at(const u32 x_, const u32 y_) const {
			assert(x_ < extent.x && y_ < extent.y);
			return texels[static_cast<size_t>(y_) * extent.x + x_];
		}
	};
}
