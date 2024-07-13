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
	vertexStage->getIntermediate()->lang.text.emplace_back(std::move(vertexShaderCode));

	/**/

	auto fragmentStage = make_smr<Stage>(StageFlagBits::eFragment);
	fragmentStage->setIntermediate(make_smr<Intermediate>());

	const auto fragmentShaderCode = read_shader_file("__test__brdf.fs");

	fragmentStage->getIntermediate()->lang.dialect = Dialect::eVulkanGlsl460;
	fragmentStage->getIntermediate()->lang.text.emplace_back(std::move(fragmentShaderCode));

	/**/

	auto tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("view", std::move(tmpVar->annotation));
	auto tmpSym = make_uptr<Symbol>(
		SymbolId::from("view"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	vertexStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eStorage>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("model", std::move(tmpVar->annotation));
	tmpSym = make_uptr<Symbol>(
		SymbolId::from("model"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	vertexStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eF32Vec3 };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("vertex-position", std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<VkBindLocationAnnotation>(
		-1L,
		0L,
		offsetof(engine::gfx::vertex, position),
		std::move(tmpVar->annotation)
	);
	vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eF32Vec3 };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("vertex-uv", std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<VkBindLocationAnnotation>(
		-1L,
		2L,
		offsetof(engine::gfx::vertex, uvm),
		std::move(tmpVar->annotation)
	);
	vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eF32Vec3 };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("vertex-normal", std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<VkBindLocationAnnotation>(
		-1L,
		3L,
		offsetof(engine::gfx::vertex, normal),
		std::move(tmpVar->annotation)
	);
	vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eU32 };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("light-info", std::move(tmpVar->annotation));
	tmpSym = make_uptr<Symbol>(
		SymbolId::from("light-info"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eStorage>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("lights", std::move(tmpVar->annotation));
	tmpSym = make_uptr<Symbol>(
		SymbolId::from("lights"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTexture2d };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("brdf-lut", std::move(tmpVar->annotation));
	tmpSym = make_uptr<Symbol>(
		SymbolId::from("brdf-lut"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	{
		auto tmpVar = make_uptr<Variable>();
		tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eSampler };
		tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
		tmpVar->annotation = make_uptr<SymbolIdAnnotation>("brdf-lut-sampler", std::move(tmpVar->annotation));
		tmpSym = make_uptr<Symbol>(
			SymbolId::from("brdf-lut-sampler"sv),
			VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
		);
		fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
		fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));
	}

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTextureCube };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("brdf-pref", std::move(tmpVar->annotation));
	tmpSym = make_uptr<Symbol>(
		SymbolId::from("brdf-pref"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	{
		auto tmpVar = make_uptr<Variable>();
		tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eSampler };
		tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
		tmpVar->annotation = make_uptr<SymbolIdAnnotation>("brdf-pref-sampler", std::move(tmpVar->annotation));
		tmpSym = make_uptr<Symbol>(
			SymbolId::from("brdf-pref-sampler"sv),
			VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
		);
		fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
		fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));
	}

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTextureCube };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("brdf-irrad", std::move(tmpVar->annotation));
	tmpSym = make_uptr<Symbol>(
		SymbolId::from("brdf-irrad"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	{
		auto tmpVar = make_uptr<Variable>();
		tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eSampler };
		tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
		tmpVar->annotation = make_uptr<SymbolIdAnnotation>("brdf-irrad-sampler", std::move(tmpVar->annotation));
		tmpSym = make_uptr<Symbol>(
			SymbolId::from("brdf-irrad-sampler"sv),
			VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
		);
		fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
		fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));
	}

	/**/

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTexture2d };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("mat-static-0"/*albedo*/, std::move(tmpVar->annotation));
	tmpSym = make_uptr<Symbol>(
		SymbolId::from("mat-static-0"sv/*albedo*/),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	{
		auto tmpVar = make_uptr<Variable>();
		tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eSampler };
		tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
		tmpVar->annotation = make_uptr<SymbolIdAnnotation>("mat-static-0-sampler", std::move(tmpVar->annotation));
		tmpSym = make_uptr<Symbol>(
			SymbolId::from("mat-static-0-sampler"sv),
			VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
		);
		fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
		fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));
	}

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTexture2d };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("mat-static-1"/*normal*/, std::move(tmpVar->annotation));
	tmpSym = make_uptr<Symbol>(
		SymbolId::from("mat-static-1"sv/*normal*/),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	{
		auto tmpVar = make_uptr<Variable>();
		tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eSampler };
		tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
		tmpVar->annotation = make_uptr<SymbolIdAnnotation>("mat-static-1-sampler", std::move(tmpVar->annotation));
		tmpSym = make_uptr<Symbol>(
			SymbolId::from("mat-static-1-sampler"sv),
			VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
		);
		fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
		fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));
	}

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTexture2d };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("mat-static-2"/*arm*/, std::move(tmpVar->annotation));
	tmpSym = make_uptr<Symbol>(
		SymbolId::from("mat-static-2"sv/*arm*/),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	{
		auto tmpVar = make_uptr<Variable>();
		tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eSampler };
		tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
		tmpVar->annotation = make_uptr<SymbolIdAnnotation>("mat-static-2-sampler", std::move(tmpVar->annotation));
		tmpSym = make_uptr<Symbol>(
			SymbolId::from("mat-static-2-sampler"sv),
			VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
		);
		fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
		fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));
	}

	/**/

	/**/

	{
		auto var = make_uptr<Variable>();
		var->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };
		var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
		var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eStorage>>(std::move(var->annotation));
		var->annotation = make_uptr<SymbolIdAnnotation>("shadows", std::move(var->annotation));

		auto sym = make_uptr<Symbol>(
			SymbolId::from("shadows"sv),
			VariableSymbol { SymbolType::eVariableSymbol, var.get() }
		);

		fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(var));
		fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(sym));
	}

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTexture2dArray };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("shadow-dir", std::move(tmpVar->annotation));
	tmpSym = make_uptr<Symbol>(
		SymbolId::from("shadow-dir"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	{
		auto tmpVar = make_uptr<Variable>();
		tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eSampler };
		tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
		tmpVar->annotation = make_uptr<SymbolIdAnnotation>("shadow-dir-sampler", std::move(tmpVar->annotation));
		tmpSym = make_uptr<Symbol>(
			SymbolId::from("shadow-dir-sampler"sv),
			VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
		);
		fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
		fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));
	}

	/**/

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("color", std::move(tmpVar->annotation));
	tmpSym = make_uptr<Symbol>(
		SymbolId::from("color"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	fragmentStage->getIntermediate()->rep.globalScope.outbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	tmpVar = make_uptr<Variable>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("depth", std::move(tmpVar->annotation));
	tmpSym = make_uptr<Symbol>(
		SymbolId::from("depth"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);
	fragmentStage->getIntermediate()->rep.globalScope.outbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	Guid guid { static_cast<u32>("__Test__Proxy"_typeId.data), 0, 0, "BrdfBaseEffect"_typeId.data };

	return make_smr<AccelerationEffect>(
		std::move(guid),
		"test-brdf-base-effect",
		Vector<smr<Stage>> { std::move(vertexStage), std::move(fragmentStage) }
	);
}

/**/

smr<const AccelerationEffect> engine::render::makeBrdfBaseEffect() {
	return build_test_effect();
}
