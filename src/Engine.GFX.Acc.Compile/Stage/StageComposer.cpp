#include "StageComposer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/AccelerationStageDerivat.hpp>

using namespace hg::engine::gfx::acc;
using namespace hg;

Vector<smr<AccelerationStageDerivat>> StageComposer::compose(
    cref<smr<AccelerationPass>> targetPass_,
    cref<ScopedTokenStorage> tokens_
) const {

    const auto& stages = targetPass_->getEffect()->getStages();

    Vector<smr<AccelerationStageDerivat>> derivats {};
    derivats.reserve(stages.size());

    for (const auto& stage : stages) {

        Vector<AccelerationStageInput> inputs { stage->getStageInputs() };
        Vector<AccelerationStageOutput> outputs { stage->getStageOutputs() };
        Vector<AccelerationStageInput> substitutionInputs {};
        Vector<AccelerationStageOutput> substitutionOutputs {};

        AccelerationStageFlagBits flagBits { stage->getFlagBits() };

        derivats.push_back(
            make_smr<AccelerationStageDerivat>(
                stage.get(),
                Vector<smr<AccelerationStage>> { stage },
                smr<AccelerationStageModule> { nullptr },
                _STD move(flagBits),
                _STD move(inputs),
                _STD move(outputs),
                _STD move(substitutionInputs),
                _STD move(substitutionOutputs)
            )
        );

    }

    return derivats;
}
