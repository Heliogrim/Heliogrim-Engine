#include "StageComposer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/AccelerationStageDerivat.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Token/Tokenizer.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

non_owning_rptr<const Tokenizer> StageComposer::getTokenizer() const noexcept {
    return _tokenizer;
}

void StageComposer::setTokenizer(mref<non_owning_rptr<const Tokenizer>> tokenizer_) {
    _tokenizer = _STD move(tokenizer_);
}

Vector<smr<AccelerationStageDerivat>> StageComposer::compose(
    cref<smr<AccelerationPass>> targetPass_,
    cref<ScopedTokenStorage> tokens_,
    cref<class SpecificationStorage> specifications_
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

        /**/

        for (auto& entry : inputs) {
            entry.token = _tokenizer->generate(entry);
        }

        for (auto& entry : outputs) {
            entry.token = _tokenizer->generate(entry);
        }

        /**/

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

    /**/

    ScopedTokenStorage bindingTokens {};
    for (const auto& derivat : derivats) {

        for (const auto& input : derivat->getStageInputs()) {
            if (input.transferType == AccelerationStageTransferType::eForward) {
                continue;
            }
            bindingTokens.addToken(clone(input.token));
        }
        for (const auto& input : derivat->substitutionInputs()) {
            if (input.transferType == AccelerationStageTransferType::eForward) {
                continue;
            }
            bindingTokens.addToken(clone(input.token));
        }

        for (const auto& output : derivat->getStageOutputs()) {
            if (output.transferType == AccelerationStageTransferType::eForward) {
                continue;
            }
            bindingTokens.addToken(clone(output.token));
        }
        for (const auto& output : derivat->substitutionOutputs()) {
            if (output.transferType == AccelerationStageTransferType::eForward) {
                continue;
            }
            bindingTokens.addToken(clone(output.token));
        }

    }

    /**/

    AccelerationPassBindings bindings {};
    for (const auto& token : bindingTokens) {

        // TODO: Query data type of binding

        AccelerationBindingLayoutElement element {};
        element.token = token;
        element.type = AccelerationBindingType::eTexture;

        AccelerationBindingLayout layout {};
        layout.elements.push_back(_STD move(element));

        bindings.layouts.push_back(_STD move(layout));
    }

    /**/

    targetPass_->setPassBindings(_STD move(bindings));

    /**/

    return derivats;
}
