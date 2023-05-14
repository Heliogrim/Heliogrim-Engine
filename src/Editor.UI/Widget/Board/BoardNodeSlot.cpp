#include "BoardNodeSlot.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/HorizontalPanel.hpp>
#include <Engine.Reflow/Widget/VerticalPanel.hpp>
#include <Engine.Reflow/Widget/Text.hpp>

#include "../../Color/Dark.hpp"
#include "Editor.GFX.Graphs/Node/NodeInput.hpp"
#include "Editor.GFX.Graphs/Node/NodeOutput.hpp"
#include "Editor.UI/Theme/Theme.hpp"

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg::editor;
using namespace hg;

BoardNodeSlot::BoardNodeSlot() :
    HorizontalPanel() {
    /**/
    attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 24.F });
    attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
    attr.colGap.setValue(2.F);
    attr.rowGap.setValue(4.F);
    attr.padding.setValue(Padding { 2.F });
}

BoardNodeSlot::~BoardNodeSlot() = default;

string BoardNodeSlot::getTag() const noexcept {
    return HorizontalPanel::getTag();
}

engine::reflow::EventResponse BoardNodeSlot::onFocus(cref<engine::reflow::FocusEvent> event_) {
    return HorizontalPanel::onFocus(event_);
}

engine::reflow::EventResponse BoardNodeSlot::onBlur(cref<engine::reflow::FocusEvent> event_) {
    return HorizontalPanel::onBlur(event_);
}

engine::reflow::EventResponse BoardNodeSlot::onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) {
    return HorizontalPanel::onMouseButtonDown(event_);
}

engine::reflow::EventResponse BoardNodeSlot::onMouseButtonUp(cref<engine::reflow::MouseEvent> event_) {
    return HorizontalPanel::onMouseButtonUp(event_);
}

engine::reflow::EventResponse BoardNodeSlot::onMouseMove(cref<engine::reflow::MouseMoveEvent> event_) {
    return HorizontalPanel::onMouseMove(event_);
}

sptr<BoardNodeSlot> BoardNodeSlot::makeInput(sptr<hg::editor::gfx::graph::NodeInput> nodeInput_) {

    const auto* const theme = Theme::get();
    auto widget = sptr<BoardNodeSlot>(new BoardNodeSlot());

    /**/

    widget->attr.justify.setValue(ReflowSpacing::eStart);

    /**/

    auto outerAnchor = make_sptr<VerticalPanel>();
    outerAnchor->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 16.F });
    outerAnchor->attr.width.setValue({ ReflowUnitType::eAbsolute, 16.F });
    outerAnchor->attr.maxWidth.setValue({ ReflowUnitType::eAbsolute, 16.F });

    outerAnchor->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 16.F });
    outerAnchor->attr.height.setValue({ ReflowUnitType::eAbsolute, 16.F });
    outerAnchor->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 16.F });

    outerAnchor->attr.padding.setValue(Padding { 1.F });

    outerAnchor->attr.style.setValue(
        PanelStyle {
            .backgroundColor = engine::color { 139.F, 0.F, 0.F, 255.F }
        }
    );

    /**/

    auto innerAnchor = make_sptr<VerticalPanel>();
    innerAnchor->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 14.F });
    innerAnchor->attr.width.setValue({ ReflowUnitType::eAbsolute, 14.F });
    innerAnchor->attr.maxWidth.setValue({ ReflowUnitType::eAbsolute, 14.F });

    innerAnchor->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 14.F });
    innerAnchor->attr.height.setValue({ ReflowUnitType::eAbsolute, 14.F });
    innerAnchor->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 14.F });

    innerAnchor->attr.padding.setValue(Padding { 8.F });

    innerAnchor->attr.style.setValue(
        PanelStyle {
            .backgroundColor = engine::color { 32.F, 36.F, 44.F, 255.F }
        }
    );

    /**/

    outerAnchor->addChild(innerAnchor);
    widget->addChild(outerAnchor);

    /**/

    auto label = make_sptr<Text>();
    theme->applyLabel(label);
    label->attr.textAlign.setValue(TextAlign::eLeftMiddle);

    label->setText(nodeInput_->_uniqueName);

    widget->addChild(label);

    /**/

    return widget;
}

sptr<BoardNodeSlot> BoardNodeSlot::makeOutput(sptr<hg::editor::gfx::graph::NodeOutput> nodeOutput_) {

    const auto* const theme = Theme::get();
    auto widget = sptr<BoardNodeSlot>(new BoardNodeSlot());

    /**/

    widget->attr.justify.setValue(ReflowSpacing::eEnd);

    /**/

    auto label = make_sptr<Text>();
    theme->applyLabel(label);
    label->attr.textAlign.setValue(TextAlign::eRightMiddle);

    label->setText(nodeOutput_->_uniqueName);

    widget->addChild(label);

    /**/

    auto outerAnchor = make_sptr<VerticalPanel>();
    outerAnchor->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 16.F });
    outerAnchor->attr.width.setValue({ ReflowUnitType::eAbsolute, 16.F });
    outerAnchor->attr.maxWidth.setValue({ ReflowUnitType::eAbsolute, 16.F });

    outerAnchor->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 16.F });
    outerAnchor->attr.height.setValue({ ReflowUnitType::eAbsolute, 16.F });
    outerAnchor->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 16.F });

    outerAnchor->attr.padding.setValue(Padding { 1.F });

    outerAnchor->attr.style.setValue(
        PanelStyle {
            .backgroundColor = engine::color { 0.F, 100.F, 0.F, 255.F }
        }
    );

    /**/

    auto innerAnchor = make_sptr<VerticalPanel>();
    innerAnchor->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 14.F });
    innerAnchor->attr.width.setValue({ ReflowUnitType::eAbsolute, 14.F });
    innerAnchor->attr.maxWidth.setValue({ ReflowUnitType::eAbsolute, 14.F });

    innerAnchor->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 14.F });
    innerAnchor->attr.height.setValue({ ReflowUnitType::eAbsolute, 14.F });
    innerAnchor->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 14.F });

    innerAnchor->attr.padding.setValue(Padding { 8.F });

    innerAnchor->attr.style.setValue(
        PanelStyle {
            .backgroundColor = engine::color { 32.F, 36.F, 44.F, 255.F }
        }
    );

    /**/

    outerAnchor->addChild(innerAnchor);
    widget->addChild(outerAnchor);

    /**/

    return widget;
}
