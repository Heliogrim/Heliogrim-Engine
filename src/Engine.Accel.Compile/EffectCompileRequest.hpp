#pragma once

#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Accel.Lang/Ir/Tier.hpp>

#include "Profile/EffectProfile.hpp"
#include "Spec/EffectSpecification.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel {
	struct EffectCompileRequest final {
		smr<const AccelerationEffect> effect;
		smr<const EffectProfile> profile;
		smr<const EffectSpecification> spec;
		#ifdef _DEBUG
		lang::ir::Tier tier = lang::ir::Tier::eDev;
		#else
		lang::ir::Tier tier = lang::ir::Tier::eBuild;
		#endif
	};
}
