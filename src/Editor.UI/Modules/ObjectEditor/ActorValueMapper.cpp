#include <Ember/Actor.hpp>
#include <Ember/ActorComponent.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Style/StyleCondition.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Input/InputFloat.hpp>
#include <Engine.Reflow/Widget/Input/InputIntegral.hpp>
#include <Engine.Reflow/Widget/Input/InputText.hpp>

#include "ObjectValueMapper.hpp"
#include "../../Color/Dark.hpp"
#include "../../Style/Style.hpp"
#include "Engine.GFX.Glow.UI/TestUI.hpp"
#include "../../Widget/Input/InputVec.hpp"

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

template <>
void ObjectValueMapper<Actor>::build(cref<sptr<engine::reflow::Box>> parent_) {

    auto actorGuid = make_sptr<Text>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TitleKey)));
    actorGuid->style().minWidth = ReflowUnit { ReflowUnitType::eRelative, 1.F };
    actorGuid->style().width = ReflowUnit { ReflowUnitType::eRelative, 1.F };
    actorGuid->style().maxWidth = ReflowUnit { ReflowUnitType::eRelative, 1.F };
    actorGuid->setText(R"(<<Actor Guid>>)");

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

    /**/

    parent_->addChild(actorGuid);
    parent_->addChild(pos);
    parent_->addChild(rot);
    parent_->addChild(scale);

}

template <>
void ObjectValueMapper<Actor>::update(cref<sptr<engine::reflow::Box>> parent_, const ptr<void> obj_) {

    const auto& actor { *static_cast<ptr<Actor>>(obj_) };
    const auto& mat { actor.getWorldTransform() };
    const auto& children { *parent_->children() };

    const auto actorGuid = actor.guid();
    _STD static_pointer_cast<Text, Widget>(children.front())->setText(
        _STD format(R"({}-{}-{}-{})", actorGuid.pre, actorGuid.c0, actorGuid.c1, actorGuid.post)
    );

    _STD static_pointer_cast<InputVec3, Widget>(children[1])->setValue(mat.position());
    _STD static_pointer_cast<InputVec3, Widget>(children[2])->setValue(mat.rotation().euler());
    _STD static_pointer_cast<InputVec3, Widget>(children[3])->setValue(mat.scale());

    /**/

    _STD static_pointer_cast<InputVec3, Widget>(children[1])->_callback = [actor = &actor](math::vec3 value_) {
        const_cast<ref<Transform>>(actor->getRootComponent()->getWorldTransform()).setPosition(_STD move(value_));
    };

    _STD static_pointer_cast<InputVec3, Widget>(children[2])->_callback = [actor = &actor](math::vec3 value_) {
        const_cast<ref<Transform>>(actor->getRootComponent()->getWorldTransform()).setRotation(
            math::quaternion::euler(value_));
    };

    _STD static_pointer_cast<InputVec3, Widget>(children[3])->_callback = [actor = &actor](math::vec3 value_) {
        const_cast<ref<Transform>>(actor->getRootComponent()->getWorldTransform()).setScale(_STD move(value_));
    };
}

template <>
void ObjectValueMapper<Actor>::cleanup(cref<sptr<engine::reflow::Box>> parent_) {
    parent_->clearChildren();
}
