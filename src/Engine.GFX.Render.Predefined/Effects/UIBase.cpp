#include "UIBase.hpp"

#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.GFX.Render.Subpass/Impl/__tmp_helper.hpp>
#include <Engine.GFX/Geometry/UIVertex.hpp>

using namespace hg::engine::render;
using namespace hg::engine::accel;
using namespace hg::engine::accel::lang;
using namespace hg;

smr<const AccelerationEffect> engine::render::makeUiBaseEffect() {

	auto vertexStage = make_smr<Stage>(StageFlagBits::eVertex);
	vertexStage->setIntermediate(make_smr<lang::Intermediate>());

	const auto vertexShaderCode = read_shader_file("__test__ui.vs");

	vertexStage->getIntermediate()->lang.dialect = Dialect::eVulkanGlsl460;
	vertexStage->getIntermediate()->lang.text.emplace_back(std::move(vertexShaderCode));

	/**/

	auto fragmentStage = make_smr<Stage>(StageFlagBits::eFragment);
	fragmentStage->setIntermediate(make_smr<lang::Intermediate>());

	const auto fragmentShaderCode = read_shader_file("__test__ui.fs");

	fragmentStage->getIntermediate()->lang.dialect = Dialect::eVulkanGlsl460;
	fragmentStage->getIntermediate()->lang.text.emplace_back(std::move(fragmentShaderCode));

	/**/

	auto tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eConstant>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("block", std::move(tmpVar->annotation));

	auto tmpSym = make_uptr<Symbol>(
		SymbolId::from("block"sv),
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
		offsetof(gfx::uivertex, position),
		std::move(tmpVar->annotation)
	);

	vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eU8Vec4 };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("vertex-color", std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<VkBindLocationAnnotation>(
		-1L,
		1L,
		offsetof(gfx::uivertex, color),
		std::move(tmpVar->annotation)
	);

	vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eF32Vec3 };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("vertex-uv", std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<VkBindLocationAnnotation>(
		-1L,
		2L,
		offsetof(gfx::uivertex, uvm),
		std::move(tmpVar->annotation)
	);

	vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTexture2d };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("ui-image", std::move(tmpVar->annotation));

	tmpSym = make_uptr<Symbol>(
		SymbolId::from("ui-image"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);

	fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	{
		auto tmpVar = make_uptr<Variable>();
		tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eSampler };
		tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
		tmpVar->annotation = make_uptr<SymbolIdAnnotation>("ui-image-sampler", std::move(tmpVar->annotation));
		tmpSym = make_uptr<Symbol>(
			SymbolId::from("ui-image-sampler"sv),
			VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
		);
		fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
		fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));
	}

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->type = Type { .category = TypeCategory::eTexture, .textureType = TextureType::eTexture2d };
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("color", std::move(tmpVar->annotation));

	tmpSym = make_uptr<Symbol>(
		SymbolId::from("color"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);

	fragmentStage->getIntermediate()->rep.globalScope.outbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	Guid guid { "__Test__Proxy"_typeId.data, 0, 0, "UIBaseEffect"_typeId.data };

	return make_smr<AccelerationEffect>(
		std::move(guid),
		"test-ui-base-effect",
		std::vector { std::move(vertexStage), std::move(fragmentStage) }
	);
}
