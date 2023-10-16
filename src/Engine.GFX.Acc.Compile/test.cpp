#include "test.h"

#include <filesystem>
#include <fstream>
#include <Engine.GFX.Acc.Compile/VkEffectCompiler.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>

#include "Spec/SimpleEffectSpecification.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

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

void hg::test_spirv_compiler() {

    const auto compiler = makeVkAccCompiler();

    const auto vertexShaderCode = read_shader_file("__test__.vs");
    const auto fragmentShaderCode = read_shader_file("__test__.fs");

    const auto vertex = make_smr<Stage>(
        StageFlagBits::eVertex,
        Vector<StageInput> {},
        Vector<StageOutput> {}
    );
    vertex->setIntermediate(
        make_smr<lang::Intermediate>(
            make_uptr<lang::IL>(
                lang::ILDialect::eVulkanGlsl,
                Vector<string> { _STD move(vertexShaderCode) }
            ),
            nullptr
        )
    );

    const auto fragment = make_smr<Stage>(
        StageFlagBits::eFragment,
        Vector<StageInput> {},
        Vector<StageOutput> {}
    );
    fragment->setIntermediate(
        make_smr<lang::Intermediate>(
            make_uptr<lang::IL>(
                lang::ILDialect::eVulkanGlsl,
                Vector<string> { _STD move(fragmentShaderCode) }
            ),
            nullptr
        )
    );

    Guid guid;
    GuidGenerate(guid);
    const auto effect = make_smr<AccelerationEffect>(
        _STD move(guid),
        "",
        Vector<smr<Stage>> { vertex, fragment },
        Vector<smr<Symbol>> {},
        Vector<smr<Symbol>> {}
    );

    const auto profile = make_smr<EffectProfile>(
        EffectProfile {
            ._name = "Default-Static-Opaque-DepthOnly",
            ._definitions = {
                ProfileDefinition { DefinitionType::eText, "#define HG_SCTX_DEPTH_ONLY\n" },
                ProfileDefinition { DefinitionType::eText, "#define HG_NAMED_MESH_TYPE 1\n" }
            }
        }
    );

    const auto spec = make_smr<SimpleEffectSpecification>(
        Vector<smr<const Symbol>> {}
    );
    spec->setPassSpec(
        make_uptr<GraphicsPassSpecification>(
            GraphicsPassSpecification {
                .depthCompareOp = DepthCompareOp::eLess,
                .stencilCompareOp = StencilCompareOp::eNever,
                .stencilFailOp = StencilOp::eKeep,
                .stencilPassOp = StencilOp::eKeep,
                .stencilDepthFailOp = StencilOp::eKeep,
                .stencilCompareMask = 0ul,
                .stencilWriteMask = 0ul,
                .primitiveTopology = PrimitiveTopology::eTriangleList,
                .renderPass = nmpt<void> {}
            }
        )
    );

    const auto pass = compiler->compile(
        EffectCompileRequest {
            .effect = effect,
            .profile = profile,
            .spec = *spec,
        }
    );

}
