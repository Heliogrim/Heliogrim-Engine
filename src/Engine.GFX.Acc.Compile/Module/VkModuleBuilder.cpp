#include "VkModuleBuilder.hpp"

#include <filesystem>
#include <regex>
#include <string>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/Pass/VkGraphicsPass.hpp>
#include <Engine.Logging/Logger.hpp>

#include "VkModuleSource.hpp"
#include "../Spec/EffectSpecification.hpp"
#include "../Token/Tokenizer.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

constexpr static const char* dummy_shader_code = "\
#version 430 core\n\
\n\
void main() {\n\
}\n\
";

[[nodiscard]] string replace_token(cref<string> token_, cref<string> replacement_, mref<string> src_);

VkModuleBuilder::~VkModuleBuilder() noexcept = default;

bool VkModuleBuilder::isFirstStage(
    cref<smr<AccelerationPass>> targetPass_,
    cref<smr<StageDerivat>> stage_
) const noexcept {

    if (targetPass_->getMetaClass()->exact<VkGraphicsPass>() &&
        stage_->getFlagBits() == StageFlagBits::eVertex
    ) {
        return true;
    }

    return false;
}

uptr<ModuleSource> VkModuleBuilder::build(
    cref<smr<AccelerationPass>> targetPass_,
    cref<EffectSpecification> specifications_,
    cref<smr<StageDerivat>> stage_,
    cref<uptr<ModuleSource>> previous_
) const {

    auto ms = make_uptr<VkModuleSource>();

    /**/

    switch (stage_->getFlagBits()) {
        case StageFlagBits::eVertex: {
            ms->targetStage = ModuleTargetStage::eVertex;
            break;
        }
        case StageFlagBits::eFragment: {
            ms->targetStage = ModuleTargetStage::eFragment;
            break;
        }
        default: {
            ms->targetStage = ModuleTargetStage::eUndefined;
            break;
        }
    }

    /* Resolve tokenized binding layouts to platform specific location bindings */

    s32 dsetIndex = 0i32;
    s32 dsetLocation = 0i32;

    // Warning: This has to be invariant over all stages within one target pass
    for (const auto& layout : targetPass_->getPassBindings().layouts) {

        for (const auto& element : layout.elements) {

            ms->mappedLocations.insert(
                _STD make_pair(
                    element.token,
                    VkBindingLocation {
                        dsetIndex,
                        dsetLocation
                    }
                )
            );

            ++dsetLocation;
        }

        dsetLocation = 0i32;
        ++dsetIndex;
    }

    /**/

    if (isFirstStage(targetPass_, stage_)) {

        // If we are the first stage within the effect pipe, we are required to generate our own input locations
        constexpr s32 invalidSet = -1i32;
        s32 location = 0i32;

        for (const auto& sib : stage_->getStageInputs()) {

            if (sib.transferType != TransferType::eForward) {
                continue;
            }

            ms->mappedLocations.insert(
                _STD make_pair(
                    sib.token,
                    VkBindingLocation {
                        invalidSet,
                        location++
                    }
                )
            );

        }

    } else {

        // If we are a follow-up stage, we need to query the previous stage for our input locations
        const auto* const prev = static_cast<const ptr<const VkModuleSource>>(previous_.get());

        for (const auto& sib : stage_->getStageInputs()) {

            if (sib.transferType != TransferType::eForward) {
                continue;
            }

            const auto outToken = _tokenizer->transformAccStageInToOut(sib.token);
            const auto prevIt = prev->mappedLocations.find(outToken);

            if (prevIt == prev->mappedLocations.end()) {
                IM_CORE_ERRORF(
                    "Failed to link forwarded data transfer `{}` -> `{}` between stages.",
                    outToken.value,
                    sib.token.value
                );
                continue;
            }

            ms->mappedLocations.insert(
                _STD make_pair(
                    sib.token,
                    prevIt->second
                )
            );

        }

    }

    /**/

    constexpr s32 invalidSet = -1i32;
    s32 location = 0i32;

    for (const auto& sob : stage_->getStageOutputs()) {

        if (sob.transferType != TransferType::eForward) {
            continue;
        }

        ms->mappedLocations.insert(
            _STD make_pair(
                sob.token,
                VkBindingLocation {
                    invalidSet,
                    location++
                }
            )
        );

    }

    /* Transform IL shader source code to platform shader source code. */

    if (not stage_->getIntermediate()) {

        auto intermediate = make_smr<lang::Intermediate>();
        auto il = make_uptr<lang::IL>();

        il->_dialect = lang::ILDialect::eVulkanGlsl;
        il->_snippets.push_back(dummy_shader_code);

        intermediate->setIl(_STD move(il));

        stage_->setIntermediate(_STD move(intermediate));
    }

    ms = transpile(stage_, _STD move(ms));

    /**/

    return ms;
}

#pragma region Vk Transpile

string VkModuleBuilder::transpile(cref<TransferToken> token_) const {
    string tmp = token_.value;

    // Change: '/' -> '_'
    // Change: '$' -> '__'

    auto it = tmp.find('/');
    while (it < tmp.size()) {
        tmp = tmp.replace(it, sizeof('/'), "_");
        it = tmp.find('/');
    }

    it = tmp.find('$');
    while (it < tmp.size()) {
        tmp = tmp.replace(it, sizeof('$'), "__");
        it = tmp.find('$');
    }

    return tmp;
}

uptr<VkModuleSource> VkModuleBuilder::transpile(
    cref<smr<StageDerivat>> stage_,
    mref<uptr<VkModuleSource>> module_
) const {

    Vector<ptr<const _STD pair<TransferToken, VkBindingLocation>>> mappings {};
    mappings.reserve(module_->mappedLocations.size());

    for (const auto& mapping : module_->mappedLocations) {
        mappings.push_back(&mapping);
    }

    _STD sort(
        mappings.begin(),
        mappings.end(),
        [](const auto& left_, const auto& right_) -> auto {
            const auto ls = left_->first.value.size();
            const auto rs = right_->first.value.size();

            return ls != rs ? ls > rs : left_->first.hash > right_->first.hash;
        }
    );

    /**/

    // Copy current il (we only support vulkan glsl currently) to preprocess
    module_->sourceCode = stage_->getIntermediate()->getIl()->_snippets;

    /**/

    for (const auto mapping : mappings) {

        const auto& token = mapping->first;
        const auto& binding = mapping->second;

        auto codes = _STD move(module_->sourceCode);
        module_->sourceCode.reserve(codes.size());

        for (auto&& code : codes) {

            /*

            layout in <type> <token> -> layout (location = ?) in <type> <token'>
            layout out <type> <token> -> layout (location = ?) out <type> <token'>

            layout <type> <token> (?<struct>) -> layout (set = ?, binding = ?) <type> <token' | token'_t> (?<struct'> <token'>)

            (layout <struct>) { <token> } -> (layout <struct>) { <type> <token'> }
            
             */

            if (_tokenizer->isStageIn(token, true)) {

                const auto ttoken = transpile(token);

                /* First, correct the layout binding */

                const _STD regex esReg { R"([-[\]{}()*+?.,\^$|#\s])" };
                const auto stoken = _STD regex_replace(token.value, esReg, R"(\$&)");

                _STD string snippet = _STD move(code);
                _STD regex reg { _STD format(R"((layout)\s+(in)\s+([a-zA-Z0-9]+)\s+({})\s*;)", stoken) };
                _STD string rep = _STD format(R"($1 (location = {}) $2 $3 {};)", binding.location, ttoken);

                auto result = _STD regex_replace(_STD move(snippet), reg, _STD move(rep));

                /* Second, replace all usages of the token */

                result = replace_token(token.value, ttoken, _STD move(result));

                /**/

                module_->sourceCode.push_back(_STD move(result));
                continue;
            }

            if (_tokenizer->isStageOut(token, true)) {

                const auto ttoken = transpile(token);

                /* First, correct the layout binding */

                const _STD regex esReg { R"([-[\]{}()*+?.,\^$|#\s])" };
                const auto stoken = _STD regex_replace(token.value, esReg, R"(\$&)");

                _STD string snippet = _STD move(code);
                _STD regex reg { _STD format(R"((layout)\s+(out)\s+([a-zA-Z0-9]+)\s+({})\s*;)", stoken) };
                _STD string rep = _STD format(R"($1 (location = {}) $2 $3 {};)", binding.location, ttoken);

                auto result = _STD regex_replace(_STD move(snippet), reg, _STD move(rep));

                /* Second, replace all usages of the token */

                result = replace_token(token.value, ttoken, _STD move(result));

                /**/

                module_->sourceCode.push_back(_STD move(result));
                continue;
            }

            if (_tokenizer->isStageIn(token, false)) {

                const auto ttoken = transpile(token);
                string snippet = _STD move(code);

                assert((binding.set >= 0) == (binding.location >= 0));

                if (binding.set >= 0 && binding.location >= 0) {

                    /* First, correct the layout binding */
                    // Warning: We need two regex replace passes due to the differences between straight types and nested types.

                    const _STD regex esReg { R"([\/-\[\]{}()*+?.,\^$|#\s])" };
                    const auto stoken = _STD regex_replace(token.value, esReg, R"(\$&)");

                    {
                        /* Straight type replacement */
                        _STD regex reg {
                            _STD format(R"((layout)\s+([a-zA-Z0-9\s]+)\s+({})(\s*;))", stoken)
                        };
                        _STD string rep = _STD format(
                            R"($1 (set = {}, binding = {}) $2 {}$4)",
                            binding.set,
                            binding.location,
                            ttoken
                        );

                        snippet = _STD regex_replace(_STD move(snippet), reg, _STD move(rep));
                    }

                    {
                        /* Nested type replacement */
                        _STD regex reg {
                            _STD format(
                                R"((layout)\s+([a-zA-Z0-9\s]+)\s+({})(\s*{}[^{}]+{})\s*;)",
                                stoken,
                                R"(\{)",
                                R"(\})",
                                R"(\})"
                            )
                        };
                        _STD string rep = _STD format(
                            R"($1 (set = {}, binding = {}) $2 {}_t$4 {};)",
                            binding.set,
                            binding.location,
                            ttoken,
                            ttoken
                        );

                        snippet = _STD regex_replace(_STD move(snippet), reg, _STD move(rep));
                    }
                }

                /* Second, replace all usages of the token */

                snippet = replace_token(token.value, ttoken, _STD move(snippet));

                /**/

                module_->sourceCode.push_back(_STD move(snippet));
                continue;
            }

            /* Fallthrough Case */
            module_->sourceCode.push_back(_STD move(code));
        }

    }

    return _STD move(module_);
}

#pragma endregion

/**/

string replace_token(cref<string> token_, cref<string> replacement_, mref<string> src_) {

    size_t off = 0;
    auto repIt = src_.find(token_, off);
    while (repIt < src_.size()) {

        const auto endIt = repIt + token_.size();
        const auto nextChar = endIt < src_.size() ? src_.at(endIt) : ' ';

        if (
            ('0' <= nextChar && nextChar <= '9') ||
            ('A' <= nextChar && nextChar <= 'Z') ||
            ('a' <= nextChar && nextChar <= 'z') ||
            nextChar == '_'
        ) {
            off = repIt;
            continue;
        }

        src_ = src_.replace(repIt, token_.size(), replacement_);
        off = repIt;
        repIt = src_.find(token_, off);
    }

    return src_;
}
