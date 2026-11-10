#pragma once

#include <Engine.Accel.Effect/AccelerationEffect.hpp>

namespace hg::engine::graphics::material {
	struct MaterialEffect {
		smr<const accel::AccelerationEffect> effect;
		smr<EffectUsagePattern> pattern;
	};
}
