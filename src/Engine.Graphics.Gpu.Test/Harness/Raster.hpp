#pragma once

#include <cassert>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Vector.hpp>

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
