#pragma once

#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::reflow {
	struct LayoutContext {
		math::vec2 localOffset;
		math::vec2 localSize;
		float localScale;
	};
}