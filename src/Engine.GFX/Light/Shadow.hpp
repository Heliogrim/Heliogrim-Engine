#pragma once
#include <Engine.Common/Math/Matrix.hpp>
#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::gfx {
	constexpr static u8 cascadeCount = 4u;

	struct alignas(alignof(float)) GlslDirectionalShadow {
		math::mat4 shadowViewProj[cascadeCount];
		f32 cascadeDepth[cascadeCount];
	};

	static_assert(alignof(GlslDirectionalShadow) == alignof(float));
	static_assert(sizeof(GlslDirectionalShadow) == sizeof(math::mat4) * cascadeCount + sizeof(f32) * cascadeCount);

	/**/

	struct alignas(alignof(u32)) GlslDirectionalShadowInfo {
		u32 shadowCount;
	};

	static_assert(sizeof(GlslDirectionalShadowInfo) == sizeof(u32));
}
