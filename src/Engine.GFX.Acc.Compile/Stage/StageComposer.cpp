#include "StageComposer.hpp"

#include <map>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/AccelerationStageDerivat.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Token/Tokenizer.hpp"
#include <Engine.GFX.Acc.Lang/Intermediate.hpp>

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

        derivats.back()->setIntermediate(make_smr<lang::Intermediate>(*stage->getIntermediate()));
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

        AccelerationBindingLayoutElement element {};
        element.token = token;

        // TODO: Query data type of binding
        const auto& descriptions = targetPass_->getEffect()->getBindingLayout().getDescriptions();
        const auto it = _STD find_if(
            descriptions.begin(),
            descriptions.end(),
            [&](const auto& description_) {
                const auto tmp = _tokenizer->generate(
                    BindingLayoutDescription {
                        description_.token,
                        AccelerationStageTransferDataType::eUnknown,
                        description_.bindingMode,
                        description_.ioMode,
                        {}
                    }
                );
                return token == tmp;
            }
        );

        if (it == descriptions.end()) {
            // TODO:
            __debugbreak();
        }

        switch (it->dataType) {
            case AccelerationStageTransferDataType::eConstant: {
                // TODO:
                __debugbreak();
                break;
            }
            case AccelerationStageTransferDataType::eSampler: {
                element.type = AccelerationBindingType::eTexture;
                break;
            }
            case AccelerationStageTransferDataType::eStorage: {
                element.type = AccelerationBindingType::eStorageBuffer;
                break;
            }
            case AccelerationStageTransferDataType::eUniform: {
                element.type = AccelerationBindingType::eUniformBuffer;
                break;
            }
            default: {
                // TODO:
                __debugbreak();
            }
        }

        /**/

        non_owning_rptr<AccelerationBindingLayout> targetLayout = nullptr;
        for (auto& layout : bindings.layouts) {
            if (layout.compatible(element)) {
                targetLayout = &layout;
                break;
            }
        }

        /**/

        if (targetLayout != nullptr) {
            targetLayout->elements.push_back(_STD move(element));
            continue;
        }

        AccelerationBindingLayout layout {};
        layout.elements.push_back(_STD move(element));
        bindings.layouts.push_back(_STD move(layout));
    }

    /**/

    targetPass_->setPassBindings(_STD move(bindings));

    /**/

    // TODO:
    /*
    layout in $bind/material/albedo;
    layout in $bind/material/albedoIdx;
    layout in $bind/material/ao;
    layout in $bind/material/aoIdx;

    // Wrong
    layout uniform $bind/material {
	    uint $bind/material/albedo
	    sampler2DArray $bind/material/albedoIdx
	    uint $bind/material/ao
	    sampler2DArray $bind/material/aoIdx
    };

    // Correct
    layout uniform sampler2DArray $bind/material/albedo;
    layout uniform sampler2DArray $bind/material/ao;

    layout uniform $bind/material {
	    uint $bind/material/albedoIdx;
	    uint $bind/material/aoIdx;
    };
     */

    return derivats;
}
