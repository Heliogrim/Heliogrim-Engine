#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Input/InputFloat.hpp>
#include <Engine.Reflow/Widget/Input/InputIntegral.hpp>
#include <Engine.Reflow/Widget/Input/InputText.hpp>
#include <Heliogrim/Heliogrim.hpp>
#include <Heliogrim/Component/Scene/Geometry/StaticGeometryComponent.hpp>

#include "ObjectValueMapper.hpp"
#include "../../Color/Dark.hpp"
#include "../../Theme/Theme.hpp"
#include "../../Widget/Collapse.hpp"
#include "../../Widget/Input/InputAsset.hpp"
#include "../../Widget/Input/InputVec.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

#pragma region Static Geometry

template <>
void ObjectValueMapper<StaticGeometryComponent>::build(cref<sptr<engine::reflow::VerticalPanel>> parent_) {

	const auto theme = Theme::get();

	auto title = make_sptr<Text>();
	theme->applyText(title);
	title->setText(R"(<<Static Geometry Component>>)");

	parent_->addChild(title);

	/**/
	auto transform { make_sptr<Collapse>() };
	transform->setup();
	transform->getHeader()->setTitle("Transform");

	parent_->addChild(transform); {
		auto wrapper = make_sptr<VerticalPanel>();

		wrapper->attr.minWidth.setValue({ ReflowUnitType::eRelative, 1.F });
		wrapper->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
		wrapper->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
		wrapper->attr.minHeight.setValue({ ReflowUnitType::eAuto, 0.F });
		wrapper->attr.height.setValue({ ReflowUnitType::eAuto, 0.F });
		wrapper->attr.colGap.setValue(4.F);
		wrapper->attr.justify.setValue(ReflowSpacing::eStart);
		wrapper->attr.flexGrow.setValue(1.F);
		wrapper->attr.flexShrink.setValue(1.F);
		wrapper->attr.style.setValue(PanelStyle { .backgroundColor = engine::color { 255.F, 20.F, 20.F, 255.F } });

		auto pos = make_sptr<InputVec3>();
		auto rot = make_sptr<InputVec3>();
		auto scale = make_sptr<InputVec3>();

		pos->setLabel(0, "X");
		pos->setLabel(1, "Y");
		pos->setLabel(2, "Z");

		rot->setLabel(0, "P");
		rot->setLabel(1, "Y");
		rot->setLabel(2, "R");

		scale->setLabel(0, "X");
		scale->setLabel(1, "Y");
		scale->setLabel(2, "Z");

		wrapper->addChild(pos);
		wrapper->addChild(rot);
		wrapper->addChild(scale);

		transform->setContent(wrapper);
	}

	/**/

	auto staticMesh { make_sptr<Collapse>() };
	staticMesh->setup();
	staticMesh->getHeader()->setTitle("Static Geometry");

	parent_->addChild(staticMesh); {
		auto wrapper = make_sptr<VerticalPanel>();

		wrapper->attr.minWidth.setValue({ ReflowUnitType::eRelative, 1.F });
		wrapper->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
		wrapper->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
		wrapper->attr.minHeight.setValue({ ReflowUnitType::eAuto, 0.F });
		wrapper->attr.height.setValue({ ReflowUnitType::eAuto, 0.F });
		wrapper->attr.colGap.setValue(4.F);

		auto assetInput { make_sptr<InputAsset>() };
		assetInput->setup();
		wrapper->addChild(assetInput);

		staticMesh->setContent(wrapper);
	}

	/**/
	auto materials { make_sptr<Collapse>() };
	materials->setup();
	materials->getHeader()->setTitle("Materials");

	parent_->addChild(materials); {
		auto wrapper = make_sptr<VerticalPanel>();

		wrapper->attr.minWidth.setValue({ ReflowUnitType::eRelative, 1.F });
		wrapper->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
		wrapper->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
		wrapper->attr.minHeight.setValue({ ReflowUnitType::eAuto, 0.F });
		wrapper->attr.height.setValue({ ReflowUnitType::eAuto, 0.F });
		wrapper->attr.colGap.setValue(4.F);

		materials->setContent(wrapper);
	}
}

template <>
void ObjectValueMapper<StaticGeometryComponent>::update(
	cref<sptr<engine::reflow::VerticalPanel>> parent_,
	const ptr<void> obj_
) {

	auto& sgc = *static_cast<ptr<StaticGeometryComponent>>(obj_);
	const auto& mat { sgc.getUniverseTransform() };
	const auto& children { *parent_->children() };

	auto* const transform { static_cast<ptr<Collapse>>(children[1].get()) }; {
		const auto wrapper = std::static_pointer_cast<VerticalPanel, Widget>(transform->getContent());

		std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(0))->setValue(
			mat.location().into()
		);
		std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(1))->setValue(mat.rotator().euler());
		std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(2))->setValue(mat.scale());

		std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(0))->_callback = [sgc = &sgc
			](math::vec3 value_) {
				const_cast<ref<Transform>>(sgc->getUniverseTransform()).setLocation(math::Location(std::move(value_)));
			};

		std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(1))->_callback = [sgc = &sgc](
			math::vec3 value_
		) {
				const_cast<ref<Transform>>(sgc->getUniverseTransform()).setRotator(
					math::Rotator::fromEuler(std::move(value_))
				);
			};

		std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(2))->_callback = [sgc = &sgc
			](math::vec3 value_) {

				const_cast<ref<Transform>>(sgc->getUniverseTransform()).setScale(std::move(value_));
			};
	}

	const auto* const staticMeshCollapse = static_cast<ptr<Collapse>>(children[2].get());
	const auto staticMeshWrap = std::static_pointer_cast<VerticalPanel, Widget>(staticMeshCollapse->getContent());
	auto* const staticMesh { static_cast<ptr<InputAsset>>(staticMeshWrap->children()->front().get()) };

	staticMesh->addAcceptedType(engine::assets::StaticGeometry::typeId);
	staticMesh->setValue(sgc.getStaticGeometryGuid());

	staticMesh->_callback = [sgc = &sgc](const asset_guid value_) {

		auto next = Heliogrim::assets()[value_];

		// TODO: Remove assert and handle invalid case ~ reset input field
		assert(next.flags & AssetDatabaseResultType::eSuccess);

		if (next.flags & AssetDatabaseResultType::eSuccess) {
			// Warning: Absolute Garbage
			sgc->setStaticGeometryByAsset(*reinterpret_cast<ptr<StaticGeometryAsset>>(&next.value));
		}
	};

	/**/

	const auto& sga { sgc.getStaticGeometryAsset() };
	const auto* const materialCollapse = static_cast<ptr<Collapse>>(children[3].get());
	const auto materials = std::static_pointer_cast<VerticalPanel, Widget>(materialCollapse->getContent());
	const auto overrides { sgc.overrideMaterials() };

	const auto materialSlotCount { materials->children()->size() };
	if (materialSlotCount > sga.getMaterialCount()) {
		const Vector<sptr<Widget>> remove {
			materials->children()->end() - (materialSlotCount - sga.getMaterialCount()),
			materials->children()->end()
		};

		for (const auto& entry : remove) {
			materials->removeChild(entry);
		}

	} else if (materialSlotCount < sga.getMaterialCount()) {

		for (auto i { materialSlotCount }; i < sga.getMaterialCount(); ++i) {
			auto slot { make_sptr<InputAsset>() };
			slot->setup();
			slot->addAcceptedType(engine::assets::GfxMaterial::typeId);

			materials->addChild(slot);
			slot->setParent(materials);
		}
	}

	for (u32 matIdx { 0uL }; matIdx < sga.getMaterialCount(); ++matIdx) {

		auto* const input = static_cast<const ptr<InputAsset>>(materials->children()->at(matIdx).get());

		input->_callback = [sgc = &sgc, matIdx](asset_guid value_) {

			//AssetDatabaseResult<GfxMaterialAsset> next = Heliogrim::assets()[value_]; // Will break due to conversion
			auto next = Heliogrim::assets()[value_];
			ref<GfxMaterialAsset> slot = const_cast<ref<AutoArray<GfxMaterialAsset>>>(sgc->overrideMaterials())[matIdx];

			// TODO: Remove assert and handle invalid case ~ reset input field ?!?
			assert(next.flags & AssetDatabaseResultType::eSuccess);

			if (next.flags & AssetDatabaseResultType::eSuccess) {
				// Error: Seams broken; Internal resolving shows that texture guids are not set and therefore crashes while trying to lock preserved resources
				// Warning: Absolute Garbage
				slot = *reinterpret_cast<ptr<GfxMaterialAsset>>(&next.value);
			}
		};

		/**/

		if (matIdx < overrides.size()) {
			input->setValue(overrides[matIdx].guid());

		} else {
			input->reset();
		}
	}
}

template <>
void ObjectValueMapper<StaticGeometryComponent>::cleanup(cref<sptr<engine::reflow::VerticalPanel>> parent_) {
	parent_->clearChildren();
}

#pragma endregion
