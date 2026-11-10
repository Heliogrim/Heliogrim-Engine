#pragma once

#include <Engine.Accel.Effect/AccelerationEffect.hpp>

namespace hg::engine::gfx::material {
	struct MaterialEffect {
		smr<const accel::AccelerationEffect> effect;
		smr<EffectUsagePattern> pattern;
	};
}
