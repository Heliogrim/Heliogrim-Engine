#include "StageComposer.hpp"

#include "StageIr.hpp"
#include "../Profile/EffectProfile.hpp"
#include "../Spec/EffectSpecification.hpp"

import Engine.Accel.Effect;
import Engine.Accel.Pipeline;
import Engine.Common;
import Engine.Logging;

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
	cref<smr<AccelerationPipeline>> targetPass_,
	const lang::ir::Tier tier_
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
			&pipeline,
			true,
			{},
			tier_
		);
		if (not composed.has_value()) {
			engine::diag::LogSink sink {};
			for (auto& diagnostic : composed.error()) {
				diagnostic.addNote("while composing effect '" + targetPass_->getEffect()->getName() + "'");
				sink.emit(std::move(diagnostic));
			}
			return {};
		}

		for (size_t idx = 0uLL; idx < derivats.size(); ++idx) {
			derivats[idx]->setIntermediate((*composed)[idx]->getIntermediate());
		}

		const auto clash = appendBindLayout(*pipeline, derivats, layout);
		if (not clash.empty()) {
			for (const auto& diagnostic : clash) {
				Logger::error("Failed to lay out ir stages: {}", diagnostic.message());
			}
			return {};
		}
	}

	/**/

	targetPass_->setBindingLayout(std::move(layout));
	return derivats;
}
