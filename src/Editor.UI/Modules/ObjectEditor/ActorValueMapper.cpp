#include <Ember/Actor.hpp>
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

    auto posX = make_sptr<InputFloat>(makeInputBoxStyle(), makeInputTextStyle());
    auto posY = make_sptr<InputFloat>(makeInputBoxStyle(), makeInputTextStyle());
    auto posZ = make_sptr<InputFloat>(makeInputBoxStyle(), makeInputTextStyle());

    /**/

    parent_->addChild(actorGuid);
    parent_->addChild(posX);
    parent_->addChild(posY);
    parent_->addChild(posZ);
}

template <>
void ObjectValueMapper<Actor>::update(cref<sptr<engine::reflow::Box>> parent_, const ptr<void> obj_) {

    const auto& actor { *static_cast<ptr<Actor>>(obj_) };
    const auto& mat { actor.getWorldTransform() };
    const auto& children { *parent_->children() };

    _STD static_pointer_cast<Text, Widget>(children.front())->setText(_STD format(R"({})", actor.guid().as_uint64()));

    _STD static_pointer_cast<InputFloat, Widget>(children[1])->setValue(mat.position().x);
    _STD static_pointer_cast<InputFloat, Widget>(children[2])->setValue(mat.position().y);
    _STD static_pointer_cast<InputFloat, Widget>(children[3])->setValue(mat.position().z);

    /**/

    _STD static_pointer_cast<InputFloat, Widget>(children[1])->_callback = [actor = &actor](
        InputFloat::input_type value_) {

            auto pos { actor->getRootComponent()->getWorldTransform().position() };
            pos.x = static_cast<decltype(pos)::value_type>(value_);
            const_cast<ref<Transform>>(actor->getRootComponent()->getWorldTransform()).setPosition(_STD move(pos));
        };

    _STD static_pointer_cast<InputFloat, Widget>(children[2])->_callback = [actor = &actor](
        InputFloat::input_type value_) {

            auto pos { actor->getRootComponent()->getWorldTransform().position() };
            pos.y = static_cast<decltype(pos)::value_type>(value_);
            const_cast<ref<Transform>>(actor->getRootComponent()->getWorldTransform()).setPosition(_STD move(pos));
        };

    _STD static_pointer_cast<InputFloat, Widget>(children[3])->_callback = [actor = &actor](
        InputFloat::input_type value_) {

            auto pos { actor->getRootComponent()->getWorldTransform().position() };
            pos.z = static_cast<decltype(pos)::value_type>(value_);
            const_cast<ref<Transform>>(actor->getRootComponent()->getWorldTransform()).setPosition(_STD move(pos));
        };
}

template <>
void ObjectValueMapper<Actor>::cleanup(cref<sptr<engine::reflow::Box>> parent_) {
    parent_->clearChildren();
}
