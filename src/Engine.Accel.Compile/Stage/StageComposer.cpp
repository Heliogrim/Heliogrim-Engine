#include "StageComposer.hpp"

#include <ranges>
#include <map>
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Profile/EffectProfile.hpp"
#include "../Profile/ProfileDefinition.hpp"

using namespace hg::engine::accel;
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

        StageFlagBits flagBits { stage->getFlagBits() };

        /**/

        derivats.push_back(
            make_smr<StageDerivat>(
                stage.get(),
                Vector<smr<Stage>> { stage },
                smr<StageModule> { nullptr },
                _STD move(flagBits)
            )
        );

        preprocessStageDerivat(stage, profile_, derivats.back());
        //derivats.back()->setIntermediate(make_smr<lang::Intermediate>(*stage->getIntermediate()));
    }

    /**/

    BindLayout layout {};

    Vector<nmpt<const lang::Symbol>> symbols {};
    for (const auto& symbol : symbols) {

        BindElement element {
            symbol
        };

        /**/

        const auto storeType = [](ref<BindElement> dst_) {

            const auto type = dst_.symbol->var.data->type;

            bool isTexture = false;
            bool isStorage = false;
            bool isUniform = false;
            bool isConstant = false;

            if (/*TransferDataType::eConstant*/isConstant) {
                // TODO:
                __debugbreak();
            } else if (isTexture) {
                //dst_.type = AccelerationBindingType::eTexture;
            } else if (isStorage) {
                //dst_.type = AccelerationBindingType::eStorageBuffer;
            } else if (isUniform) {
                //dst_.type = AccelerationBindingType::eUniformBuffer;
            } else {
                // TODO:
                __debugbreak();
            }
        };

        /**/

        assert(symbol->var.type == lang::SymbolType::eVariableSymbol);
        assert(symbol->var.data->annotation);

        bool hasExternalLinkage = false;

        auto* annotation = symbol->var.data->annotation.get();
        while (annotation != nullptr) {
            if (annotation->type == lang::AnnotationType::eExternalLinkage) {
                hasExternalLinkage = true;
                break;
            }
            annotation = annotation->next.get();
        }

        /**/

        if (not hasExternalLinkage) {
            continue;
        }

        storeType(element);

        /**/

        bool grouped = false;
        for (auto& candidate : layout.groups) {
            if (/* candidate.compatible(element) */false) {
                candidate.elements.push_back(_STD move(element));
                grouped = true;
                break;
            }
        }

        if (not grouped) {
            auto group = BindGroup {};
            group.elements.push_back(_STD move(element));
            layout.groups.push_back(_STD move(group));
        }
    }

    /**/

    targetPass_->setBindingLayout(_STD move(layout));
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

    auto& il = intermediate->lang.text;
    _STD reverse(il.begin(), il.end());

    /* Store injections and external definitions in reverse order */
    il.emplace_back(shaderMacros);

    for (auto iter = profile_->_definitions.rbegin(); iter != profile_->_definitions.rend(); ++iter) {

        const auto& definition = *iter;
        if (definition.type == DefinitionType::eText) {
            il.emplace_back(definition.data);
        }
    }

    for (const auto& module : profile_->_modules | _STD views::reverse) {

        // TODO: Merge intermediate representations
        intermediate->rep;
        module.rep;

        // Temporary alternative
        for (const auto block : module.lang.text | _STD views::reverse) {
            il.emplace_back(block);
        }
    }

    il.emplace_back(_STD move(preamble));
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
    _STD reverse(il.begin(), il.end());

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
