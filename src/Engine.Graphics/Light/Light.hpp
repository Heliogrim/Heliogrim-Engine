#pragma once
#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::gfx {
	struct alignas(alignof(float)) GlslLight {
		// color * intensity
		math::fvec4 luminance;
		math::fvec4 position;
		math::fvec4 direction;

		union {
			float limit;
			math::fvec4 dummy;
		};
	};

	//static_assert(sizeof(GlslLight) == sizeof(float) * 10);
	static_assert(sizeof(GlslLight) == sizeof(float) * 16);

	/**/

	struct alignas(alignof(u32)) GlslSceneLightInfo {
		u32 lightCount;
	};

	static_assert(sizeof(GlslSceneLightInfo) == sizeof(u32));
}
