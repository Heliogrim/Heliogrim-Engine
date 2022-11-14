#include <Ember/StaticGeometryComponent.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Style/StyleCondition.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Input/InputFloat.hpp>
#include <Engine.Reflow/Widget/Input/InputIntegral.hpp>
#include <Engine.Reflow/Widget/Input/InputText.hpp>
#include <Engine.Session/Session.hpp>
#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Assets/Database/AssetDatabaseQuery.hpp>
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>

#include "Engine.GFX.Glow.UI/TestUI.hpp"

#include "ObjectValueMapper.hpp"
#include "../../Color/Dark.hpp"
#include "../../Style/Style.hpp"
#include "../../Widget/Input/InputVec.hpp"
#include "Ember/Ember.hpp"

using namespace ember::editor::ui;
using namespace ember::engine::reflow;
using namespace ember;

[[nodiscard]] static sptr<BoundStyleSheet> makeInputBoxStyle() {

    auto style {
        BoundStyleSheet::make(StyleSheet {
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
        })

    };

    style->pushStyle({
        Style::key_type::from("InputType::Focused"),
        style::isFocused,
        make_sptr<StyleSheet>(StyleSheet {
            .color { true, color::Dark::backgroundInnerFieldDarken }
        })
    });

    return style;

}

[[nodiscard]] static sptr<BoundStyleSheet> makeInputTextStyle() {
    auto* font { getDefaultFont() };

    auto style {
        BoundStyleSheet::make(StyleSheet {
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
        })
    };

    style->pushStyle({
        Style::key_type::from("InputType::Focused"),
        style::isNever,
        make_sptr<StyleSheet>(StyleSheet {
            .color { true, color::Dark::white }
        })
    });

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

    auto pos = make_sptr<InputVec3>();
    auto rot = make_sptr<InputVec3>();
    auto scale = make_sptr<InputVec3>();

    auto assetGuid = make_sptr<InputIntegral>(makeInputBoxStyle(), makeInputTextStyle());

    pos->setLabel(0, "X");
    pos->setLabel(1, "Y");
    pos->setLabel(2, "Z");

    rot->setLabel(0, "P");
    rot->setLabel(1, "Y");
    rot->setLabel(2, "R");

    scale->setLabel(0, "X");
    scale->setLabel(1, "Y");
    scale->setLabel(2, "Z");

    /**/

    parent_->addChild(title);

    parent_->addChild(pos);
    parent_->addChild(rot);
    parent_->addChild(scale);

    parent_->addChild(assetGuid);
}

template <>
void ObjectValueMapper<StaticGeometryComponent>::update(cref<sptr<engine::reflow::Box>> parent_, const ptr<void> obj_) {

    const ref<StaticGeometryComponent> sgc { *static_cast<ptr<StaticGeometryComponent>>(obj_) };
    const auto& mat { sgc.getWorldTransform() };
    const auto& children { *parent_->children() };

    _STD static_pointer_cast<InputVec3, Widget>(children[1])->setValue(mat.position());
    _STD static_pointer_cast<InputVec3, Widget>(children[2])->setValue(mat.rotation().euler());
    _STD static_pointer_cast<InputVec3, Widget>(children[3])->setValue(mat.scale());

    _STD static_pointer_cast<InputIntegral, Widget>(children[4])->setValue(sgc.getStaticGeometryGuid().as_uint64());

    /**/

    _STD static_pointer_cast<InputVec3, Widget>(children[1])->_callback = [sgc = &sgc](math::vec3 value_) {
        const_cast<ref<Transform>>(sgc->getWorldTransform()).setPosition(_STD move(value_));
    };

    _STD static_pointer_cast<InputVec3, Widget>(children[2])->_callback = [sgc = &sgc](math::vec3 value_) {
        const_cast<ref<Transform>>(sgc->getWorldTransform()).setRotation(
            math::quaternion::euler(value_));
    };

    _STD static_pointer_cast<InputVec3, Widget>(children[3])->_callback = [sgc = &sgc](math::vec3 value_) {

        const_cast<ref<Transform>>(sgc->getWorldTransform()).setScale(_STD move(value_));
    };

    /**/

    _STD static_pointer_cast<InputIntegral, Widget>(children[4])->_callback = [sgc = &sgc](const s64 value_) {

        const auto session { engine::Session::get() };
        const auto db { session->modules().assetDatabase() };

        asset_guid guid {};
        guid.as_uint64() = value_;
        const auto query { db->query(guid) };

        if (not query.exists()) {
            return;
        }

        if (query.get()->getTypeId() != engine::assets::StaticGeometry::typeId) {
            return;
        }

        const auto dbe { Ember::assets().operator[]<StaticGeometryAsset>(query.guid()) };
        sgc->setStaticGeometryByAsset(dbe.value);
    };
}

template <>
void ObjectValueMapper<StaticGeometryComponent>::cleanup(cref<sptr<engine::reflow::Box>> parent_) {
    parent_->clearChildren();
}

#pragma endregion
