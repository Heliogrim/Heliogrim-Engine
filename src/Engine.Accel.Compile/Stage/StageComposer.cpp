#include "StageComposer.hpp"

#include <map>
#include <ranges>

#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Accel.Pipeline/Stage/StageModule.hpp>
#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "StageIr.hpp"
#include "../Profile/EffectProfile.hpp"
#include "../Profile/ProfileDefinition.hpp"
#include "../Spec/EffectSpecification.hpp"

using namespace hg::engine::accel;
using namespace hg;

/**/

non_owning_rptr<const Tokenizer> StageComposer::getTokenizer() const noexcept {
	return _tokenizer;
}

void StageComposer::setTokenizer(mref<non_owning_rptr<const Tokenizer>> tokenizer_) {
	_tokenizer = std::move(tokenizer_);
}

Vector<smr<StageDerivat>> StageComposer::compose(
	cref<smr<const EffectProfile>> profile_,
	cref<smr<const EffectSpecification>> specifications_,
	cref<smr<AccelerationPipeline>> targetPass_
) const {

	const auto& stages = targetPass_->getEffect()->getStages();

	Vector<smr<StageDerivat>> derivats {};
	derivats.reserve(stages.size());

	Vector<const lang::Intermediate*> irStages {};
	Vector<StageFlagBits> irFlags {};

	for (const auto& stage : stages) {

		const auto source = stage->getIntermediate();
		if (source == nullptr || source->ir == nullptr) {
			Logger::error(
				"Failed to compose effect '{}': stage {} has no ir intermediate (legacy text stages are not supported).",
				targetPass_->getEffect()->getName(),
				static_cast<u32>(stage->getFlagBits())
			);
			return {};
		}

		StageFlagBits flagBits { stage->getFlagBits() };

		derivats.push_back(
			make_smr<StageDerivat>(
				stage.get(),
				Vector<smr<Stage>> { stage },
				smr<StageModule> { nullptr },
				std::move(flagBits)
			)
		);

		irStages.emplace_back(source.get());
		irFlags.emplace_back(stage->getFlagBits());
	}

	/* one layout for all stages; the last of them is sliced to the effect's exports */
	BindLayout layout {};

	if (not irStages.empty()) {
		smr<const lang::ir::ResolvedPipeline> pipeline {};
		auto composed = composePipelineIr(
			irStages,
			irFlags,
			*profile_,
			*specifications_,
			_variants,
			_pipelineLayouts,
			&pipeline
		);
		if (not composed.has_value()) {
			for (const auto& diagnostic : composed.error()) {
				Logger::error(
					"Failed to compose ir stage: {} ({}:{})",
					diagnostic.message,
					diagnostic.line,
					diagnostic.column
				);
			}
			return {};
		}

		for (size_t idx = 0uLL; idx < derivats.size(); ++idx) {
			derivats[idx]->setIntermediate((*composed)[idx]->getIntermediate());
		}

		const auto clash = appendBindLayout(*pipeline, derivats, layout);
		if (not clash.empty()) {
			for (const auto& diagnostic : clash) {
				Logger::error("Failed to lay out ir stages: {}", diagnostic.message);
			}
			return {};
		}
	}

	/**/

	targetPass_->setBindingLayout(std::move(layout));
	return derivats;
}
