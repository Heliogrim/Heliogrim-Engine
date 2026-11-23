#pragma once

#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>
#include <Engine.Accel.Pipeline/Stage/StageDerivat.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Lang;
import Engine.Common;
#endif

namespace hg::engine::accel {
	class StageComposer {
	public:
		using this_type = StageComposer;

	public:
		StageComposer() noexcept = default;

		~StageComposer() noexcept = default;

	private:
		non_owning_rptr<const class Tokenizer> _tokenizer;

		/* resolved ir stage variants, shared by every compose of this composer */
		mutable lang::ir::VariantCache _variants;

		/* laid out and lowered ir pipelines, keyed by their stages' variants */
		mutable lang::ir::PipelineLayoutCache _pipelineLayouts;

	public:
		[[nodiscard]] non_owning_rptr<const class Tokenizer> getTokenizer() const noexcept;

		void setTokenizer(mref<non_owning_rptr<const class Tokenizer>> tokenizer_);

	public:
		[[nodiscard]] Vector<smr<StageDerivat>> compose(
			cref<smr<const class EffectProfile>> profile_,
			cref<smr<const class EffectSpecification>> specifications_,
			cref<smr<AccelerationPipeline>> targetPass_,
			lang::ir::Tier tier_
		) const;
	};
}
