#include "ContextMenu.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/VBox.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>

using namespace ember::engine::reflow;
using namespace ember::editor::ui;
using namespace ember;

[[nodiscard]] static sptr<BoundStyleSheet> makeStyleSheet() {

    auto style = BoundStyleSheet::make(StyleSheet {
        .maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 4.F } },
        .maxHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .wrap = { true, ReflowWrap::eNoWrap },
        .padding = { true, Padding { 0.F, 2.F } },
        .margin = { true, Margin { 0.F } },
        .reflowSpacing = { true, ReflowSpacing::eStart },
    });

    return style;
}

ContextMenu::ContextMenu() :
    Menu() {}

ContextMenu::~ContextMenu() = default;

void ContextMenu::addMenuAction(mref<sptr<Widget>> menuAction_) {
    _STD static_pointer_cast<VBox, Widget>(getContent())->addChild(_STD move(menuAction_));
}

void ContextMenu::removeMenuAction(cref<sptr<Widget>> menuAction_) {
    _STD static_pointer_cast<VBox, Widget>(getContent())->removeChild(menuAction_);
}

sptr<ContextMenu> ContextMenu::make(const ptr<Widget> target_, cref<math::ivec2> position_) {

    if (!target_) {
        return nullptr;
    }

    const auto self { make_sptr<ContextMenu>() };
    const auto root { target_->root() };

    /*
    self->setAnchor(math::vec2 {
        static_cast<const math::vec2::value_type&>(position_.x),
        static_cast<const math::vec2::value_type&>(position_.y)
    });
    self->setVisible(true);
     */

    /**/

    const auto content { make_sptr<VBox>(makeStyleSheet()) };
    self->setContent(content);

    /**/

    return self;
}
