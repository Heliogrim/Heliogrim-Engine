#include "StageComposer.hpp"

#include <map>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/Stage/StageDerivat.hpp>
#include <Engine.GFX.Acc/Symbol/Symbol.hpp>
#include <Engine.GFX.Acc.Lang/Intermediate.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Token/ScopedTokenStorage.hpp"
#include "../Token/Tokenizer.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

non_owning_rptr<const Tokenizer> StageComposer::getTokenizer() const noexcept {
    return _tokenizer;
}

void StageComposer::setTokenizer(mref<non_owning_rptr<const Tokenizer>> tokenizer_) {
    _tokenizer = _STD move(tokenizer_);
}

Vector<smr<StageDerivat>> StageComposer::compose(
    cref<smr<AccelerationPass>> targetPass_,
    cref<class SpecificationStorage> specifications_
) const {

    const auto& stages = targetPass_->getEffect()->getStages();

    Vector<smr<StageDerivat>> derivats {};
    derivats.reserve(stages.size());

    for (const auto& stage : stages) {

        Vector<StageInput> inputs { stage->getStageInputs() };
        Vector<StageOutput> outputs { stage->getStageOutputs() };
        Vector<StageInput> substitutionInputs {};
        Vector<StageOutput> substitutionOutputs {};

        StageFlagBits flagBits { stage->getFlagBits() };

        /**/

        for (auto& entry : inputs) {
            entry.token = _tokenizer->generate(entry);
        }

        for (auto& entry : outputs) {
            entry.token = _tokenizer->generate(entry);
        }

        /**/

        derivats.push_back(
            make_smr<StageDerivat>(
                stage.get(),
                Vector<smr<Stage>> { stage },
                smr<StageModule> { nullptr },
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
            if (input.transferType == TransferType::eForward) {
                continue;
            }
            bindingTokens.addToken(clone(input.token));
        }
        for (const auto& input : derivat->substitutionInputs()) {
            if (input.transferType == TransferType::eForward) {
                continue;
            }
            bindingTokens.addToken(clone(input.token));
        }

        for (const auto& output : derivat->getStageOutputs()) {
            if (output.transferType == TransferType::eForward) {
                continue;
            }
            bindingTokens.addToken(clone(output.token));
        }
        for (const auto& output : derivat->substitutionOutputs()) {
            if (output.transferType == TransferType::eForward) {
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

        /**/

        const auto storeType = [](const auto& obj_, ref<AccelerationBindingLayoutElement> dst_) {
            switch (obj_.dataType) {
                case TransferDataType::eConstant: {
                    // TODO:
                    __debugbreak();
                    break;
                }
                case TransferDataType::eSampler: {
                    dst_.type = AccelerationBindingType::eTexture;
                    break;
                }
                case TransferDataType::eStorage: {
                    dst_.type = AccelerationBindingType::eStorageBuffer;
                    break;
                }
                case TransferDataType::eUniform: {
                    dst_.type = AccelerationBindingType::eUniformBuffer;
                    break;
                }
                default: {
                    // TODO:
                    __debugbreak();
                }
            }
        };

        /**/

        const auto& imported = targetPass_->getEffect()->getImportSymbols();
        const auto& exported = targetPass_->getEffect()->getExportSymbols();

        smr<Symbol> symbol { nullptr };
        if (symbol.empty()) {
            for (const auto& is : imported) {

                const auto tmp = _tokenizer->transformAccStageIn(is->token, false);
                if (tmp != token) {
                    continue;
                }

                const auto input = targetPass_->getEffect()->getFirstInputFor(*is);
                if (input.has_value() && input.value().transferType == TransferType::eBinding) {
                    symbol = clone(is);
                    storeType(input.value(), element);
                    break;
                }
            }
        }

        if (symbol.empty()) {
            for (const auto& es : exported) {

                const auto tmp = _tokenizer->transformAccStageOut(es->token, false);
                if (tmp != token) {
                    continue;
                }

                const auto output = targetPass_->getEffect()->getLastOutputFor(*es);
                if (output.has_value() && output.value().transferType == TransferType::eBinding) {
                    symbol = clone(es);
                    storeType(output.value(), element);
                    break;
                }
            }
        }

        if (symbol.empty()) {
            /* Could happen if type is TransferType::eForwarding */
            __debugbreak();
            break;
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
