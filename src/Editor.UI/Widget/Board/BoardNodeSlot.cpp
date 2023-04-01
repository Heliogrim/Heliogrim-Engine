#include "BoardNodeSlot.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/HBox.hpp>
#include <Engine.Reflow/Widget/VBox.hpp>
#include <Engine.Reflow/Widget/Text.hpp>

#include "../../Color/Dark.hpp"
#include "../../Style/Style.hpp"
#include "Editor.GFX.Graphs/Node/NodeInput.hpp"
#include "Editor.GFX.Graphs/Node/NodeOutput.hpp"

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg::editor;
using namespace hg;

/**/

static sptr<BoundStyleSheet> makeSlotStyle();

static sptr<BoundStyleSheet> makeSlotAnchorStyle();

static sptr<BoundStyleSheet> makeSlotLabelStyle();

/**/

BoardNodeSlot::BoardNodeSlot() :
    HBox(makeSlotStyle()) {}

BoardNodeSlot::~BoardNodeSlot() = default;

string BoardNodeSlot::getTag() const noexcept {
    return HBox::getTag();
}

engine::reflow::EventResponse BoardNodeSlot::onFocus(cref<engine::reflow::FocusEvent> event_) {
    return HBox::onFocus(event_);
}

engine::reflow::EventResponse BoardNodeSlot::onBlur(cref<engine::reflow::FocusEvent> event_) {
    return HBox::onBlur(event_);
}

engine::reflow::EventResponse BoardNodeSlot::onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) {
    return HBox::onMouseButtonDown(event_);
}

engine::reflow::EventResponse BoardNodeSlot::onMouseButtonUp(cref<engine::reflow::MouseEvent> event_) {
    return HBox::onMouseButtonUp(event_);
}

engine::reflow::EventResponse BoardNodeSlot::onMouseMove(cref<engine::reflow::MouseMoveEvent> event_) {
    return HBox::onMouseMove(event_);
}

sptr<BoardNodeSlot> BoardNodeSlot::makeInput(sptr<hg::editor::gfx::graph::NodeInput> nodeInput_) {

    auto widget = sptr<BoardNodeSlot>(new BoardNodeSlot());

    /**/

    widget->style()->reflowSpacing = ReflowSpacing::eStart;

    /**/

    auto outerAnchor = make_sptr<VBox>(makeSlotAnchorStyle());
    auto innerAnchor = make_sptr<VBox>(makeSlotAnchorStyle());

    outerAnchor->style()->color = color::Dark::darkGreen;
    outerAnchor->style()->padding = Padding { 1.F };

    innerAnchor->style()->width = ReflowUnit { ReflowUnitType::eAbsolute, 14.F };
    innerAnchor->style()->maxWidth = ReflowUnit { ReflowUnitType::eAbsolute, 14.F };
    innerAnchor->style()->height = ReflowUnit { ReflowUnitType::eAbsolute, 14.F };
    innerAnchor->style()->maxHeight = ReflowUnit { ReflowUnitType::eAbsolute, 14.F };
    innerAnchor->style()->borderRadius = { true, Padding { 7.F } };

    innerAnchor->style()->color = color::Dark::backgroundInnerFieldDarken;

    outerAnchor->addChild(innerAnchor);
    widget->addChild(outerAnchor);

    /**/

    auto label = make_sptr<Text>(makeSlotLabelStyle());

    label->setText(nodeInput_->_uniqueName);
    label->style().textAlign = TextAlign::eLeftMiddle;

    widget->addChild(label);

    /**/

    return widget;
}

sptr<BoardNodeSlot> BoardNodeSlot::makeOutput(sptr<hg::editor::gfx::graph::NodeOutput> nodeOutput_) {

    auto widget = sptr<BoardNodeSlot>(new BoardNodeSlot());

    /**/

    widget->style()->reflowSpacing = ReflowSpacing::eEnd;

    /**/

    auto label = make_sptr<Text>(makeSlotLabelStyle());

    label->setText(nodeOutput_->_uniqueName);
    label->style().textAlign = TextAlign::eRightMiddle;

    widget->addChild(label);

    /**/

    auto outerAnchor = make_sptr<VBox>(makeSlotAnchorStyle());
    auto innerAnchor = make_sptr<VBox>(makeSlotAnchorStyle());

    outerAnchor->style()->color = color::Dark::darkRed;
    outerAnchor->style()->padding = Padding { 1.F };

    innerAnchor->style()->width = ReflowUnit { ReflowUnitType::eAbsolute, 14.F };
    innerAnchor->style()->maxWidth = ReflowUnit { ReflowUnitType::eAbsolute, 14.F };
    innerAnchor->style()->height = ReflowUnit { ReflowUnitType::eAbsolute, 14.F };
    innerAnchor->style()->maxHeight = ReflowUnit { ReflowUnitType::eAbsolute, 14.F };
    innerAnchor->style()->borderRadius = { true, Padding { 7.F } };

    innerAnchor->style()->color = color::Dark::backgroundInnerFieldDarken;

    outerAnchor->addChild(innerAnchor);
    widget->addChild(outerAnchor);

    /**/

    return widget;
}

/**/

sptr<BoundStyleSheet> makeSlotStyle() {
    return BoundStyleSheet::make(
        StyleSheet {
            .minWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 24.F } },
            .width = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
            .height = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .rowGap = { true, ReflowUnit { ReflowUnitType::eAbsolute, 4.F } },
            .padding = { true, Padding { 2.F } },
            .color = { true, color::Dark::backgroundDefault }
        }
    );
}

sptr<BoundStyleSheet> makeSlotAnchorStyle() {
    return BoundStyleSheet::make(
        StyleSheet {
            .width = { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
            .maxWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
            .height = { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
            .maxHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
            .borderRadius = { true, Padding { 8.F } },
            .color = { true, color::Dark::black },
        }
    );
}

sptr<BoundStyleSheet> makeSlotLabelStyle() {

    auto style = BoundStyleSheet::make(
        StyleSheet {
            .height = { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
            .maxHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
        }
    );

    /**/

    style->pushStyle(
        StyleChainEntry {
            Style::TitleSmallKey,
            nullptr,
            Style::get()->getStyleSheet(Style::TitleSmallKey)
        }
    );

    /**/

    return style;
}
