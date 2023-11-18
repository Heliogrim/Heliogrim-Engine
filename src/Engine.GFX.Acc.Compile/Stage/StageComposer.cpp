#include "StageComposer.hpp"

#include <map>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/Stage/StageDerivat.hpp>
#include <Engine.GFX.Acc/Symbol/Symbol.hpp>
#include <Engine.GFX.Acc.Lang/Intermediate.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Token/ScopedTokenStorage.hpp"
#include "../Token/Tokenizer.hpp"
#include "Engine.GFX.Acc.Compile/Profile/EffectProfile.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

static void preprocessStageDerivat(
    cref<smr<const Stage>> stage_,
    cref<smr<const class EffectProfile>> profile_,
    _Inout_ smr<StageDerivat> derivat_
);

static string read_shader_file(string name_);

/**/

non_owning_rptr<const Tokenizer> StageComposer::getTokenizer() const noexcept {
    return _tokenizer;
}

void StageComposer::setTokenizer(mref<non_owning_rptr<const Tokenizer>> tokenizer_) {
    _tokenizer = _STD move(tokenizer_);
}

Vector<smr<StageDerivat>> StageComposer::compose(
    cref<smr<const EffectProfile>> profile_,
    cref<smr<const EffectSpecification>> specifications_,
    cref<smr<AccelerationPipeline>> targetPass_
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

        preprocessStageDerivat(stage, profile_, derivats.back());
        //derivats.back()->setIntermediate(make_smr<lang::Intermediate>(*stage->getIntermediate()));
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

    AccelerationPipelineBindings bindings {};
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

        smr<const Symbol> symbol { nullptr };
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

#include <sstream>

static void preprocessStageDerivat(
    cref<smr<const Stage>> stage_,
    cref<smr<const EffectProfile>> profile_,
    smr<StageDerivat> derivat_
) {

    static string shaderMacros = read_shader_file("__macros__.glsl");

    std::stringstream ss {};
    ss << "#version 450 core\n";

    switch (stage_->getFlagBits()) {
        case StageFlagBits::eVertex: {
            ss << "#define HG_SCTX_TYPE_VERTEX\n";
            break;
        }
        case StageFlagBits::eTessellationCtrl: {
            ss << "#define HG_SCTX_TYPE_TESSELLATION_CTRL\n";
            break;
        }
        case StageFlagBits::eTessellationEval: {
            ss << "#define HG_SCTX_TYPE_TESSELLATION_EVAL\n";
            break;
        }
        case StageFlagBits::eGeometry: {
            ss << "#define HG_SCTX_TYPE_GEOMETRY\n";
            break;
        }
        case StageFlagBits::eFragment: {
            ss << "#define HG_SCTX_TYPE_FRAGMENT\n";
            break;
        }
        case StageFlagBits::eCompute: {
            ss << "#define HG_SCTX_TYPE_COMPUTE\n";
            break;
        }
        default: {
            __debugbreak();
        }
    }

    auto preamble = ss.str();

    /**/

    auto intermediate = make_smr<lang::Intermediate>(*stage_->getIntermediate());

    auto& il = const_cast<lang::IL&>(intermediate->getIl().load());
    _STD reverse(il._snippets.begin(), il._snippets.end());

    /* Store injections and external definitions in reverse order */
    il._snippets.push_back(shaderMacros);

    for (auto iter = profile_->_definitions.rbegin(); iter != profile_->_definitions.rend(); ++iter) {

        const auto& definition = *iter;
        if (definition.type == DefinitionType::eText) {
            il._snippets.push_back(definition.data);
        }
    }

    il._snippets.push_back(_STD move(preamble));
    /**/

    /*
     * Layout Inversion:
     *
     * | Preamble               |
     * | ---------------------- |
     * | Profile Definitions    |
     * | ---------------------- |
     * | Shader Macros          |
     * | Shader Body            |
     *
     */
    _STD reverse(il._snippets.begin(), il._snippets.end());

    derivat_->setIntermediate(_STD move(intermediate));
}

#include <filesystem>
#include <fstream>

static string read_shader_file(string name_) {

    const auto root = R"(R:\Development\C++\Vulkan API\Game\resources\shader\)";
    std::filesystem::path file { root };
    file.append(name_);

    if (not std::filesystem::exists(file)) {
        __debugbreak();
        return {};
    }

    auto ifs = _STD ifstream(file, std::ios_base::in | std::ios_base::binary);

    ifs.seekg(0, _STD ios::end);
    const auto fsize = ifs.tellg();
    ifs.seekg(0, _STD ios::beg);

    string tmp {};
    tmp.resize(fsize);

    ifs.read(tmp.data(), fsize);
    assert(!ifs.bad());

    ifs.close();
    return tmp;
}
