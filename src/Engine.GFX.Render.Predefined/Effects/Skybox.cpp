#include "Skybox.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.Render.Subpass/Impl/__tmp_helper.hpp>

#include "Engine.GFX.Render.Predefined/Symbols/SceneCamera.hpp"
#include "Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp"
#include "Engine.GFX.Render.Predefined/Symbols/SkyboxTexture.hpp"

using namespace hg::engine::render;
using namespace hg;
using namespace ::hg::engine::accel;

static smr<AccelerationEffect> build_test_effect() {

    auto vertexStage = make_smr<Stage>(
        StageFlagBits::eVertex
        // Vector<StageInput> {
        //     StageInput {
        //         // TransferToken::from("camera"), TransferType::eBinding, TransferDataType::eUniform, DataInputRate::ePerPassInvocation
        //     }
        // },
        // Vector<StageOutput> {
        //     StageOutput {
        //         // TransferToken {}, TransferType::eForward, TransferDataType::eF32Vec3, DataOutputRate {}
        //     }
        // }
    );
    vertexStage->setIntermediate(make_smr<lang::Intermediate>());

    auto tmpVar = make_uptr<lang::Variable>();
    tmpVar->type = lang::Type { .category = lang::TypeCategory::eObject, .objectType = lang::ObjectType::eUnknown };
    tmpVar->annotation = make_uptr<lang::SimpleAnnotation<lang::AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<lang::SimpleAnnotation<lang::AnnotationType::eUniform>>(
        _STD move(tmpVar->annotation)
    );
    tmpVar->annotation = make_uptr<lang::SymbolIdAnnotation>("view", _STD move(tmpVar->annotation));
    auto tmpSym = make_uptr<lang::Symbol>(
        lang::SymbolId::from("view"),
        lang::VariableSymbol { lang::SymbolType::eVariableSymbol, tmpVar.get() }
    );
    vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    vertexStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    tmpVar = make_uptr<lang::Variable>();
    tmpVar->annotation = make_uptr<lang::SymbolIdAnnotation>("uv");
    tmpSym = make_uptr<lang::Symbol>(
        lang::SymbolId::from("uv"),
        lang::VariableSymbol { lang::SymbolType::eVariableSymbol, tmpVar.get() }
    );
    vertexStage->getIntermediate()->rep.globalScope.outbound.emplace_back(_STD move(tmpVar));
    vertexStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    auto fragmentStage = make_smr<Stage>(
        StageFlagBits::eFragment
        // Vector<StageInput> {
        //     StageInput {
        //         // TransferToken {}, TransferType::eForward, TransferDataType::eF32Vec3, DataInputRate::ePerInvocation
        //     },
        //     StageInput {
        //         // TransferToken::from("depth"), TransferType::eForward, TransferDataType::eF32, DataInputRate::ePerInvocation
        //     },
        //     StageInput {
        //         // TransferToken::from("skybox"), TransferType::eBinding, TransferDataType::eSampler, DataInputRate::ePerPassInvocation
        //     }
        // },
        // Vector<StageOutput> {
        //     StageOutput {
        //         // TransferToken {}, TransferType::eForward, TransferDataType::eU8Vec4, DataOutputRate {}
        //     }
        // }
    );
    fragmentStage->setIntermediate(make_smr<lang::Intermediate>());

    tmpVar = make_uptr<lang::Variable>();
    tmpVar->annotation = make_uptr<lang::SymbolIdAnnotation>("uv");
    tmpSym = make_uptr<lang::Symbol>(
        lang::SymbolId::from("uv"),
        lang::VariableSymbol { lang::SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    tmpVar = make_uptr<lang::Variable>();
    tmpVar->annotation = make_uptr<lang::SymbolIdAnnotation>("depth");
    tmpSym = make_uptr<lang::Symbol>(
        lang::SymbolId::from("depth"),
        lang::VariableSymbol { lang::SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    tmpVar = make_uptr<lang::Variable>();
    tmpVar->type = lang::Type {
        .category = lang::TypeCategory::eTexture, .textureType = lang::TextureType::eTexture2d
    };
    tmpVar->annotation = make_uptr<lang::SimpleAnnotation<lang::AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<lang::SimpleAnnotation<lang::AnnotationType::eUniform>>(
        _STD move(tmpVar->annotation)
    );
    tmpVar->annotation = make_uptr<lang::SymbolIdAnnotation>("mat-static-0", _STD move(tmpVar->annotation));
    tmpSym = make_uptr<lang::Symbol>(
        lang::SymbolId::from(/*"skybox"*/"mat-static-0"),
        lang::VariableSymbol { lang::SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    tmpVar = make_uptr<lang::Variable>();
    tmpVar->annotation = make_uptr<lang::SymbolIdAnnotation>("color");
    tmpSym = make_uptr<lang::Symbol>(
        lang::SymbolId::from("color"),
        lang::VariableSymbol { lang::SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.outbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    const auto vertexShaderCode = read_shader_file("__test__sky.vs");

    vertexStage->getIntermediate()->lang.dialect = lang::Dialect::eVulkanGlsl460;
    vertexStage->getIntermediate()->lang.text.emplace_back(_STD move(vertexShaderCode));

    const auto fragmentShaderCode = read_shader_file("__test__sky.fs");

    fragmentStage->getIntermediate()->lang.dialect = lang::Dialect::eVulkanGlsl460;
    fragmentStage->getIntermediate()->lang.text.emplace_back(_STD move(fragmentShaderCode));

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
        Vector<smr<Stage>> { _STD move(vertexStage), _STD move(fragmentStage) }
    );
    // Vector<smr<const Symbol>> { makeSceneCameraSymbol(), makeSkyboxTextureSymbol(), /*makeSceneDepthSymbol()*/ },
    // Vector<smr<const Symbol>> { makeSceneColorSymbol() }
}

/**/

smr<const AccelerationEffect> engine::render::makeSkyboxEffect() {
    return build_test_effect();
}
