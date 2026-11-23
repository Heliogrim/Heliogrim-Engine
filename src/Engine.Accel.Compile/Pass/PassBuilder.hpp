#pragma once
#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Effect;
#endif

namespace hg::engine::accel {
	class macro_novtable PassBuilder {
	public:
		using this_type = PassBuilder;

	public:
		PassBuilder() noexcept = default;

		virtual ~PassBuilder() noexcept = default;

	public:
		[[nodiscard]] _Success_(return != nullptr) virtual smr<AccelerationPipeline> build(
			mref<smr<const AccelerationEffect>> effect_,
			cref<smr<const class EffectSpecification>> specifications_,
			cref<smr<const class EffectProfile>> profile_
		) const noexcept = 0;
	};
}
