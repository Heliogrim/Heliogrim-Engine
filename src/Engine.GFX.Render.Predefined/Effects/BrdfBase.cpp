#include "BrdfBase.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Subpass/Impl/__tmp_helper.hpp>
#include <Engine.GFX/Geometry/Vertex.hpp>

using namespace ::hg::engine::render;
using namespace ::hg::engine::accel;
using namespace ::hg::engine::accel::lang;
using namespace ::hg;

static smr<AccelerationEffect> build_test_effect() {

    auto vertexStage = make_smr<Stage>(StageFlagBits::eVertex);
    vertexStage->setIntermediate(make_smr<Intermediate>());

    const auto vertexShaderCode = read_shader_file("__test__brdf.vs");

    vertexStage->getIntermediate()->lang.dialect = Dialect::eVulkanGlsl460;
    vertexStage->getIntermediate()->lang.text.emplace_back(_STD move(vertexShaderCode));

    /**/

    auto fragmentStage = make_smr<Stage>(StageFlagBits::eFragment);
    fragmentStage->setIntermediate(make_smr<Intermediate>());

    const auto fragmentShaderCode = read_shader_file("__test__brdf.fs");

    fragmentStage->getIntermediate()->lang.dialect = Dialect::eVulkanGlsl460;
    fragmentStage->getIntermediate()->lang.text.emplace_back(_STD move(fragmentShaderCode));

    /**/

    auto tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(_STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("view", _STD move(tmpVar->annotation));
    auto tmpSym = make_uptr<Symbol>(
        SymbolId::from("view"),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    vertexStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eStorage>>(_STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("model", _STD move(tmpVar->annotation));
    tmpSym = make_uptr<Symbol>(
        SymbolId::from("model"),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    vertexStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eF32Vec3 };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("vertex-position", _STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<VkBindLocationAnnotation>(
        -1L,
        0L,
        offsetof(engine::gfx::vertex, position),
        _STD move(tmpVar->annotation)
    );
    vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eF32Vec3 };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("vertex-uv", _STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<VkBindLocationAnnotation>(
        -1L,
        2L,
        offsetof(engine::gfx::vertex, uvm),
        _STD move(tmpVar->annotation)
    );
    vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eF32Vec3 };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("vertex-normal", _STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<VkBindLocationAnnotation>(
        -1L,
        3L,
        offsetof(engine::gfx::vertex, normal),
        _STD move(tmpVar->annotation)
    );
    vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));

    /**/

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eU32 };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(_STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("light-info", _STD move(tmpVar->annotation));
    tmpSym = make_uptr<Symbol>(
        SymbolId::from("light-info"),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eStorage>>(_STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("lights", _STD move(tmpVar->annotation));
    tmpSym = make_uptr<Symbol>(
        SymbolId::from("lights"),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTexture2d };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(_STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("brdf-lut", _STD move(tmpVar->annotation));
    tmpSym = make_uptr<Symbol>(
        SymbolId::from("brdf-lut"),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTextureCube };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(_STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("brdf-pref", _STD move(tmpVar->annotation));
    tmpSym = make_uptr<Symbol>(
        SymbolId::from("brdf-pref"),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTextureCube };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(_STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("brdf-irrad", _STD move(tmpVar->annotation));
    tmpSym = make_uptr<Symbol>(
        SymbolId::from("brdf-irrad"),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    /**/

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTexture2d };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(_STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("mat-static-0"/*albedo*/, _STD move(tmpVar->annotation));
    tmpSym = make_uptr<Symbol>(
        SymbolId::from("mat-static-0"/*albedo*/),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTexture2d };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(_STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("mat-static-1"/*normal*/, _STD move(tmpVar->annotation));
    tmpSym = make_uptr<Symbol>(
        SymbolId::from("mat-static-1"/*normal*/),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTexture2d };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(_STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("mat-static-2"/*arm*/, _STD move(tmpVar->annotation));
    tmpSym = make_uptr<Symbol>(
        SymbolId::from("mat-static-2"/*arm*/),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    /**/

    tmpVar = make_uptr<Variable>();
    tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTexture2d };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(_STD move(tmpVar->annotation));
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("shadow-dir", _STD move(tmpVar->annotation));
    tmpSym = make_uptr<Symbol>(
        SymbolId::from("shadow-dir"),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    /**/

    tmpVar = make_uptr<Variable>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("color", _STD move(tmpVar->annotation));
    tmpSym = make_uptr<Symbol>(
        SymbolId::from("color"),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.outbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    tmpVar = make_uptr<Variable>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>("depth", _STD move(tmpVar->annotation));
    tmpSym = make_uptr<Symbol>(
        SymbolId::from("depth"),
        VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
    );
    fragmentStage->getIntermediate()->rep.globalScope.outbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    Guid guid {};
    //GuidGenerate(guid);
    guid.data = uint128_t {
        "__Test__Proxy"_typeId.data,
        "BrdfBaseEffect"_typeId.data
    };

    return make_smr<AccelerationEffect>(
        _STD move(guid),
        "test-brdf-base-effect",
        Vector<smr<Stage>> { _STD move(vertexStage), _STD move(fragmentStage) }
    );
}

/**/

smr<const AccelerationEffect> engine::render::makeBrdfBaseEffect() {
    return build_test_effect();
}
