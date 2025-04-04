#pragma once

#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "../Module/CompiledModule.hpp"

namespace hg::engine::accel {
	class macro_novtable PassCompiler {
	public:
		using this_type = PassCompiler;

	public:
		PassCompiler() noexcept = default;

		virtual ~PassCompiler() noexcept = default;

	protected:
		non_owning_rptr<const class Tokenizer> _tokenizer;

	public:
		[[nodiscard]] non_owning_rptr<const class Tokenizer> getTokenizer() const noexcept;

		void setTokenizer(mref<non_owning_rptr<const class Tokenizer>> tokenizer_);

	public:
		[[nodiscard]] _Success_(return != nullptr) virtual smr<const AccelerationPipeline> compile(
			cref<smr<const class EffectSpecification>> specifications_,
			mref<smr<AccelerationPipeline>> source_,
			mref<Vector<smr<StageDerivat>>> stages_,
			mref<Vector<uptr<class CompiledModule>>> modules_
		) const = 0;
	};
}
