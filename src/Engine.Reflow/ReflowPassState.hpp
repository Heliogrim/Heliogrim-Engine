#pragma once

#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Graphics/Aabb.hpp>

namespace hg::engine::reflow {
	struct ReflowPassState {
		using Aabb2d = hg::engine::graphics::Aabb2d;

		/* Bottom-Up pass requested size without relative resolving */
		math::vec2 prefetchMinSize;
		math::vec2 prefetchSize;

		/* Bottom-up result before passPrefetchSizing resolves it, which is not idempotent */
		math::vec2 rawPrefetchMinSize;
		math::vec2 rawPrefetchSize;

		/* Forwarded Top-Down pass with relative resolving and compound arrangement */
		math::vec2 referenceSize;
		math::vec2 prefetchMaxSize;

		/* Computational size request */
		math::vec2 computeSize;

		/* Inputs of the last computeSizing per axis */
		math::vec2 cachedReferenceSize { -1.F };
		math::vec2 cachedComputeSize { -1.F };

		/* Assigned layout parameters */
		math::vec2 layoutOffset;
		math::vec2 layoutSize;

		/* Two-dimensional AABB of the last layout cycle */
		Aabb2d lastAabb { math::fvec2 { -1.F }, math::fvec2 { -1.F } };
	};
}
