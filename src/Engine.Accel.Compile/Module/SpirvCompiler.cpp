#include "SpirvCompiler.hpp"

#include <fstream>

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Logging/Logger.hpp>

// glslang includes
#include <sstream>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

using namespace hg::engine::accel;
using namespace hg;

std::atomic_uint_fast32_t hg::engine::accel::extern_compiler_ref_count { 0ui32 };

/**/

SpirvCompiler::SpirvCompiler() :
    _srcLang(SpirvSrcLang::eGlsl),
    _dialect(SpirvDialect::eVulkan),
    _targetClient(SpirvTargetClient::eVulkan13),
    _targetVersion(SpirvTargetVersion::eSpv16) {
    if (extern_compiler_ref_count.fetch_add(1ui32) == 0) {
        glslang::InitializeProcess();
    }
}

SpirvCompiler::~SpirvCompiler() {
    if (extern_compiler_ref_count.fetch_sub(1ui32) == 1ui32) {
        glslang::FinalizeProcess();
    }
}

SpirvByteCode SpirvCompiler::compile(cref<ModuleSource> module_, cref<InlineAutoArray<lang::TextBlock>> source_) const {

    const auto targetStage = static_cast<std::underlying_type_t<ModuleTargetStage>>(module_.targetStage) - 1;
    assert(targetStage >= 0 && targetStage < EShLanguageMaskCount);

    EShLanguage stage { reinterpret_cast<cref<EShLanguage>>(targetStage) };
    bool status = true;

    /**/

    uptr<glslang::TProgram> glslProgram = make_uptr<glslang::TProgram>();
    uptr<glslang::TShader> glslShader = make_uptr<glslang::TShader>(stage);

    glslProgram->addShader(glslShader.get());

    /**/

    s32 langVersion { 100 /* EShClientVulkan, 100 => SPIR-V */ };

    glslShader->setEnvClient(
        reinterpret_cast<cref<glslang::EShClient>>(_dialect),
        reinterpret_cast<cref<glslang::EShTargetClientVersion>>(_targetClient)
    );
    glslShader->setEnvInput(
        reinterpret_cast<cref<glslang::EShSource>>(_srcLang),
        stage,
        reinterpret_cast<cref<glslang::EShClient>>(_dialect),
        langVersion
    );
    glslShader->setEnvTarget(
        glslang::EShTargetLanguage::EShTargetSpv,
        reinterpret_cast<cref<glslang::EShTargetLanguageVersion>>(_targetVersion)
    );

    Vector<ptr<const char>> codeSnippets {};
    Vector<s32> codeSnippetLength {};

    codeSnippets.reserve(source_.size());
    codeSnippetLength.reserve(source_.size());

    for (const auto& textBlock : source_) {
        codeSnippets.push_back(textBlock.data.data());
        codeSnippetLength.push_back(static_cast<s32>(textBlock.data.size()));
    }

    glslShader->setStringsWithLengths(
        codeSnippets.data(),
        codeSnippetLength.data(),
        static_cast<s32>(source_.size())
    );

    /**/

    s32 defaultVersion { 110 };
    EShMessages msg { EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules };

    // Warning: Currently broken, when `setDebugInfo(true)`, SPIRV Remapping will crash
    //glslShader->setDebugInfo(false);
    //glslShader->setEnhancedMsgs();

    std::string preprocessed {};
    glslang::TShader::ForbidIncluder includer {};
    glslShader->preprocess(
        GetDefaultResources(),
        defaultVersion,
        ECoreProfile,
        false,
        false,
        msg,
        &preprocessed,
        includer
    );
    status = glslShader->parse(GetDefaultResources(), defaultVersion, false, msg);

    if (not status) {
        IM_CORE_WARN(glslShader->getInfoLog());
        IM_CORE_WARN(glslShader->getInfoDebugLog());
    }

    /**/

    if (status && not glslProgram->link(msg)) {
        IM_CORE_WARN(glslProgram->getInfoLog());
        IM_CORE_WARN(glslProgram->getInfoDebugLog());

        status = false;
    }

    /**/

    SpirvByteCode byteCode {};

    if (status) {

        const auto* const intermediate = glslProgram->getIntermediate(stage);
        glslang::SpvOptions spvOpts {
            .generateDebugInfo = true,
            .stripDebugInfo = false,
            .validate = true
        };
        spv::SpvBuildLogger spvLog {};

        glslang::GlslangToSpv(*intermediate, reinterpret_cast<ref<Vector<u32>>>(byteCode), &spvLog, &spvOpts);

        auto errorMsg = spvLog.getAllMessages();
        //if (not spvLog.errors.empty()) {
        if (not errorMsg.empty()) {
            //for (auto&& error : spvLog.errors) {
            //    IM_CORE_ERROR(std::move(error));
            //}
            IM_CORE_ERROR(std::move(errorMsg));
            __debugbreak();
            status = false;
            byteCode.clear();
        }
    }

    /**/

    glslProgram.reset();
    glslShader.reset();

    /**/

    return byteCode;
}
