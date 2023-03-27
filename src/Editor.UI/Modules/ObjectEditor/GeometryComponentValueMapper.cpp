#include <Heliogrim/StaticGeometryComponent.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Style/StyleCondition.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Input/InputFloat.hpp>
#include <Engine.Reflow/Widget/Input/InputIntegral.hpp>
#include <Engine.Reflow/Widget/Input/InputText.hpp>
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>

#include "Engine.GFX.Glow.UI/TestUI.hpp"

#include "ObjectValueMapper.hpp"
#include "../../Color/Dark.hpp"
#include "../../Style/Style.hpp"
#include "../../Widget/Input/InputVec.hpp"
#include "../../Widget/Collapse.hpp"
#include "Editor.UI/Widget/Input/InputAsset.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Heliogrim/Heliogrim.hpp"
#include "Engine.Assets/Types/GfxMaterial.hpp"
#include "Engine.Assets.System/IAssetRegistry.hpp"
#include "Engine.Core/Engine.hpp"
#include "Engine.Reflect/Cast.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

[[nodiscard]] static sptr<BoundStyleSheet> makeInputBoxStyle() {

    auto style {
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .width { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .height { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .maxHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .wrap { true, ReflowWrap::eNoWrap },
                .padding { true, Padding { 4.F, 2.F } },
                .margin { true, Margin { 0.F } },
                .reflowShrink { true, 0.F },
                .reflowGrow { true, 0.F },
                .borderRadius = { true, BorderRadius { 4.F } },
                .color { false, color::Dark::backgroundInnerField }
            }
        )

    };

    style->pushStyle(
        {
            Style::key_type::from("InputType::Focused"),
            style::isFocused,
            make_sptr<StyleSheet>(
                StyleSheet {
                    .color { true, color::Dark::backgroundInnerFieldDarken }
                }
            )
        }
    );

    return style;

}

[[nodiscard]] static sptr<BoundStyleSheet> makeInputTextStyle() {
    auto* font { getDefaultFont() };

    auto style {
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .width { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                .height { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                .maxHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                .color { false, color::Dark::grey },
                .font { true, font },
                .fontSize { true, 16.F },
                .textAlign { true, TextAlign::eLeftMiddle },
                .textEllipse { true, 1ui32 }
            }
        )
    };

    style->pushStyle(
        {
            Style::key_type::from("InputType::Focused"),
            style::isNever,
            make_sptr<StyleSheet>(
                StyleSheet {
                    .color { true, color::Dark::white }
                }
            )
        }
    );

    return style;
}

#pragma region Static Geometry

template <>
void ObjectValueMapper<StaticGeometryComponent>::build(cref<sptr<engine::reflow::Box>> parent_) {

    auto title = make_sptr<Text>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TitleKey)));
    title->style().minWidth = ReflowUnit { ReflowUnitType::eRelative, 1.F };
    title->style().width = ReflowUnit { ReflowUnitType::eRelative, 1.F };
    title->style().maxWidth = ReflowUnit { ReflowUnitType::eRelative, 1.F };
    title->setText(R"(<<Static Geometry Component>>)");

    parent_->addChild(title);

    /**/
    auto transform { make_sptr<Collapse>() };
    transform->setup();
    transform->getHeader()->setTitle("Transform");

    parent_->addChild(transform);

    {
        const auto wrap { transform->getContent() };
        wrap->style()->minHeight = ReflowUnit { ReflowUnitType::eAuto, 0.F };
        wrap->style()->height = ReflowUnit { ReflowUnitType::eAuto, 0.F };
        wrap->style()->wrap = ReflowWrap::eNoWrap;
        wrap->style()->colGap = ReflowUnit { ReflowUnitType::eAbsolute, 4.F };
        wrap->style()->color = color::Dark::backgroundDefault;

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

        wrap->addChild(pos);
        wrap->addChild(rot);
        wrap->addChild(scale);
    }

    /**/

    auto staticMesh { make_sptr<Collapse>() };
    staticMesh->setup();
    staticMesh->getHeader()->setTitle("Static Geometry");

    parent_->addChild(staticMesh);

    {
        const auto wrap { staticMesh->getContent() };
        wrap->style()->minHeight = ReflowUnit { ReflowUnitType::eAuto, 0.F };
        wrap->style()->height = ReflowUnit { ReflowUnitType::eAuto, 0.F };
        wrap->style()->wrap = ReflowWrap::eNoWrap;
        wrap->style()->colGap = ReflowUnit { ReflowUnitType::eAbsolute, 4.F };
        wrap->style()->color = color::Dark::backgroundDefault;

        auto assetInput { make_sptr<InputAsset>() };
        wrap->addChild(assetInput);
    }

    /**/
    auto materials { make_sptr<Collapse>() };
    materials->setup();
    materials->getHeader()->setTitle("Materials");

    parent_->addChild(materials);

    {
        const auto wrap { materials->getContent() };
        wrap->style()->minHeight = ReflowUnit { ReflowUnitType::eAuto, 0.F };
        wrap->style()->height = ReflowUnit { ReflowUnitType::eAuto, 0.F };
        wrap->style()->wrap = ReflowWrap::eNoWrap;
        wrap->style()->colGap = ReflowUnit { ReflowUnitType::eAbsolute, 4.F };
        wrap->style()->color = color::Dark::backgroundDefault;
    }
}

template <>
void ObjectValueMapper<StaticGeometryComponent>::update(cref<sptr<engine::reflow::Box>> parent_, const ptr<void> obj_) {

    const ref<StaticGeometryComponent> sgc { *static_cast<ptr<StaticGeometryComponent>>(obj_) };
    const auto& mat { sgc.getWorldTransform() };
    const auto& children { *parent_->children() };

    auto* const transform { static_cast<ptr<Collapse>>(children[1].get()) };

    {
        const auto wrap { transform->getContent() };

        _STD static_pointer_cast<InputVec3, Widget>((*wrap->children())[0])->setValue(
            mat.location().operator math::fvec3()
        );
        _STD static_pointer_cast<InputVec3, Widget>((*wrap->children())[1])->setValue(mat.rotator().euler());
        _STD static_pointer_cast<InputVec3, Widget>((*wrap->children())[2])->setValue(mat.scale());

        _STD static_pointer_cast<InputVec3, Widget>((*wrap->children())[0])->_callback = [sgc = &sgc
            ](math::vec3 value_) {
                const_cast<ref<Transform>>(sgc->getWorldTransform()).setLocation(math::Location(_STD move(value_)));
            };

        _STD static_pointer_cast<InputVec3, Widget>((*wrap->children())[1])->_callback = [sgc = &sgc](
            math::vec3 value_
        ) {
                const_cast<ref<Transform>>(sgc->getWorldTransform()).setRotator(
                    math::Rotator::fromEuler(_STD move(value_))
                );
            };

        _STD static_pointer_cast<InputVec3, Widget>((*wrap->children())[2])->_callback = [sgc = &sgc
            ](math::vec3 value_) {

                const_cast<ref<Transform>>(sgc->getWorldTransform()).setScale(_STD move(value_));
            };
    }

    const auto staticMeshWrap { static_cast<ptr<Collapse>>(children[2].get())->getContent() };
    const auto staticMesh { static_cast<ptr<InputAsset>>(staticMeshWrap->children()->front().get()) };

    staticMesh->addAcceptedType(engine::assets::StaticGeometry::typeId);
    staticMesh->setValue(sgc.getStaticGeometryGuid());

    staticMesh->_callback = [sgc = &sgc](asset_guid value_) {

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
    const auto materials { _STD static_pointer_cast<Collapse, Widget>(children[3])->getContent() };
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
            slot->addAcceptedType(engine::assets::GfxMaterial::typeId);
            materials->addChild(slot);
        }
    }

    for (u32 matIdx { 0ui32 }; matIdx < sga.getMaterialCount(); ++matIdx) {

        auto* const input = static_cast<const ptr<InputAsset>>((*materials->children())[matIdx].get());

        input->_callback = [sgc = &sgc, matIdx](asset_guid value_) {

            //AssetDatabaseResult<GfxMaterialAsset> next = Heliogrim::assets()[value_]; // Will break due to conversion
            auto next = Heliogrim::assets()[value_];
            ref<GfxMaterialAsset> slot = const_cast<ref<Vector<GfxMaterialAsset>>>(sgc->overrideMaterials())[matIdx];

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
void ObjectValueMapper<StaticGeometryComponent>::cleanup(cref<sptr<engine::reflow::Box>> parent_) {
    parent_->clearChildren();
}

#pragma endregion
