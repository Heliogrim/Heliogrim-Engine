#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Effect;
#endif

namespace hg::engine::graphics::material {
	struct MaterialEffect {
		smr<const accel::AccelerationEffect> effect;
		smr<EffectUsagePattern> pattern;
	};
}
