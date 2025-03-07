#pragma once

#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.GFX/Aabb.hpp>

namespace hg::engine::reflow {
	struct ReflowPassState {
		using Aabb2d = hg::engine::gfx::Aabb2d;

		/* Bottom-Up pass requested size without relative resolving */
		math::vec2 prefetchMinSize;
		math::vec2 prefetchSize;

		/* Forwarded Top-Down pass with relative resolving and compound arrangement */
		math::vec2 referenceSize;
		math::vec2 prefetchMaxSize;

		/* Computational size request */
		math::vec2 computeSize;

		/* Assigned layout parameters */
		math::vec2 layoutOffset;
		math::vec2 layoutSize;

		/* Two-dimensional AABB of the last layout cycle */
		Aabb2d lastAabb { math::fvec2 { -1.F }, math::fvec2 { -1.F } };
	};
}
