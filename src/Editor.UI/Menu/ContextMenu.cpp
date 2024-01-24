#include "ContextMenu.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/VerticalPanel.hpp>

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg;

ContextMenu::ContextMenu() :
    Menu() {}

ContextMenu::~ContextMenu() = default;

void ContextMenu::addMenuAction(mref<sptr<Widget>> menuAction_) {
    std::static_pointer_cast<VerticalPanel, Widget>(getContent())->addChild(std::move(menuAction_));
}

void ContextMenu::removeMenuAction(cref<sptr<Widget>> menuAction_) {
    std::static_pointer_cast<VerticalPanel, Widget>(getContent())->removeChild(menuAction_);
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

    const auto content = make_sptr<VerticalPanel>();
    content->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    content->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 4.F });
    content->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
    content->attr.padding.setValue(Padding { 0.F, 2.F });
    content->attr.justify.setValue(ReflowSpacing::eStart);
    self->setContent(content);

    /**/

    return self;
}
