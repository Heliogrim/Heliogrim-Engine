#pragma once
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>
#include <Engine.Common/Sal.hpp>

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
