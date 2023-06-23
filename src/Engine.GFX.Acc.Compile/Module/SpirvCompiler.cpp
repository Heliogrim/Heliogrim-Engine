#include "SpirvCompiler.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>

// glslang includes
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

// spirv includes
#include <spirv-tools/libspirv.hpp>
#include <spirv-tools/linker.hpp>
#include <spirv-tools/optimizer.hpp>

using namespace hg::engine::gfx::acc;
using namespace hg;

_STD atomic_uint_fast32_t hg::engine::gfx::acc::extern_compiler_ref_count { 0ui32 };

/**/

SpirvCompiler::SpirvCompiler() :
    _srcLang(SpirvSrcLang::eGlsl),
    _dialect(SpirvDialect::eVulkan),
    _targetClient(SpirvTargetClient::eVulkan13),
    _targetVersion(SpirvTargetVersion::eSpv16) {
    if (extern_compiler_ref_count.fetch_add(1ui32) == 0) {
        ShInitialize();
        glslang::InitializeProcess();
    }
}

SpirvCompiler::~SpirvCompiler() {
    if (extern_compiler_ref_count.fetch_sub(1ui32) == 1ui32) {
        ShFinalize();
        glslang::FinalizeProcess();
    }
}

SpirvByteCode SpirvCompiler::compile(cref<ModuleSource> module_, cref<Vector<string>> source_) const {

    const auto targetStage = static_cast<_STD underlying_type_t<ModuleTargetStage>>(module_.targetStage) - 1;
    assert(targetStage >= 0 && targetStage < EShLanguageMaskCount);

    EShLanguage stage { reinterpret_cast<cref<EShLanguage>>(targetStage) };
    bool status = true;

    /**/

    uptr<glslang::TProgram> glslProgram = make_uptr<glslang::TProgram>();
    uptr<glslang::TShader> glslShader = make_uptr<glslang::TShader>(stage);

    glslProgram->addShader(glslShader.get());

    /**/

    s32 langVersion { 100 /* 450 */ };

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

    for (u32 i = 0; i < source_.size(); ++i) {
        codeSnippets.push_back(source_[i].data());
        codeSnippetLength.push_back(static_cast<s32>(source_[i].size()));
    }

    glslShader->setStringsWithLengths(codeSnippets.data(), codeSnippetLength.data(), static_cast<s32>(source_.size()));

    /**/

    s32 defaultVersion { 110 };
    EShMessages msg { EShMsgDefault };

    status = glslShader->parse(GetResources(), defaultVersion, false, msg);

    /**/

    if (status) {
        status = glslProgram->link(msg);
    }

    /**/

    SpirvByteCode byteCode {};

    if (status) {

        const auto* const intermediate = glslProgram->getIntermediate(stage);
        glslang::SpvOptions spvOpts {};
        spv::SpvBuildLogger spvLog {};

        glslang::GlslangToSpv(*intermediate, reinterpret_cast<ref<Vector<u32>>>(byteCode), &spvLog, &spvOpts);
    }

    /**/

    glslProgram.reset();
    glslShader.reset();

    /**/

    return byteCode;
}
