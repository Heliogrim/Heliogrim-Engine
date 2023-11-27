#include "Skybox.hpp"

#include <Engine.Common/Make.hpp>

#include "Engine.GFX.Render.Predefined/Symbols/SceneCamera.hpp"
#include "Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp"
#include "Engine.GFX.Render.Predefined/Symbols/SkyboxTexture.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

#include <filesystem>
#include <fstream>
using namespace ::hg::engine::gfx::acc;

static string read_shader_file(string name_) {

    const auto root = R"(R:\Development\C++\Vulkan API\Game\resources\shader\)";
    std::filesystem::path file { root };
    file.append(name_);

    if (not exists(file)) {
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

smr<AccelerationEffect> build_test_effect() {

    auto vertexStage = make_smr<Stage>(
        StageFlagBits::eVertex,
        Vector<StageInput> {
            StageInput {
                TransferToken::from("camera"), TransferType::eBinding, TransferDataType::eUniform,
                DataInputRate::ePerPassInvocation
            }
        },
        Vector<StageOutput> {
            StageOutput {
                TransferToken {}, TransferType::eForward, TransferDataType::eF32Vec3,
                DataOutputRate {}
            }
        }
    );

    auto fragmentStage = make_smr<Stage>(
        StageFlagBits::eFragment,
        Vector<StageInput> {
            StageInput {
                TransferToken {}, TransferType::eForward, TransferDataType::eF32Vec3,
                DataInputRate::ePerInvocation
            },
            StageInput {
                TransferToken::from("depth"), TransferType::eForward, TransferDataType::eF32,
                DataInputRate::ePerInvocation
            },
            StageInput {
                TransferToken::from("skybox"), TransferType::eBinding, TransferDataType::eSampler,
                DataInputRate::ePerPassInvocation
            }
        },
        Vector<StageOutput> {
            StageOutput {
                TransferToken {}, TransferType::eForward, TransferDataType::eU8Vec4,
                DataOutputRate {}
            }
        }
    );

    /**/

    const auto vertexShaderCode = read_shader_file("__test__sky.vs");

    vertexStage->setIntermediate(
        make_smr<lang::Intermediate>(
            make_uptr<lang::IL>(lang::ILDialect::eVulkanGlsl, Vector<string> { _STD move(vertexShaderCode) }),
            nullptr
        )
    );

    const auto fragmentShaderCode = read_shader_file("__test__sky.fs");

    fragmentStage->setIntermediate(
        make_smr<lang::Intermediate>(
            make_uptr<lang::IL>(
                lang::ILDialect::eVulkanGlsl,
                Vector<string> { _STD move(fragmentShaderCode) }
            ),
            nullptr
        )
    );

    /**/

    Guid guid {};
    //GuidGenerate(guid);
    guid.data = uint128_t {
        "__Test__Proxy"_typeId.data,
        "SkyboxEffect"_typeId.data
    };

    return make_smr<AccelerationEffect>(
        _STD move(guid),
        "test-sky-effect",
        Vector<smr<Stage>> { _STD move(vertexStage), _STD move(fragmentStage) },
        Vector<smr<const Symbol>> { makeSceneCameraSymbol(), makeSkyboxTextureSymbol(), /*makeSceneDepthSymbol()*/ },
        Vector<smr<const Symbol>> { makeSceneColorSymbol() }
    );
}

/**/

smr<const engine::gfx::acc::AccelerationEffect> engine::gfx::render::makeSkyboxEffect() {
    return build_test_effect();
}
