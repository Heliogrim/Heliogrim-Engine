#include "BoardNode.hpp"

#include "../../Color/Dark.hpp"
#include "Editor.UI/Style/Style.hpp"
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/HBox.hpp>
#include <Engine.Reflow/Widget/Text.hpp>

#include <Editor.GFX.Graphs/Node/Node.hpp>
#include "BoardNodeSlot.hpp"

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg::editor;
using namespace hg;

/**/

static sptr<BoundStyleSheet> makeBoardNodeStyle();

/**/

static sptr<BoundStyleSheet> makeHeaderStyle();

static sptr<BoundStyleSheet> makeHeaderSymbolStyle();

static sptr<BoundStyleSheet> makeHeaderBoxStyle();

static sptr<BoundStyleSheet> makeHeaderTitleStyle();

static sptr<BoundStyleSheet> makeHeaderSubtitleStyle();

/**/

static sptr<BoundStyleSheet> makeBodyStyle();

static sptr<BoundStyleSheet> makeBodyTripletStyle();

/**/

BoardNode::BoardNode() :
    VBox(makeBoardNodeStyle()) {}

BoardNode::~BoardNode() = default;

string BoardNode::getTag() const noexcept {
    return VBox::getTag();
}

engine::reflow::EventResponse BoardNode::onFocus(cref<engine::reflow::FocusEvent> event_) {
    return VBox::onFocus(event_);
}

engine::reflow::EventResponse BoardNode::onBlur(cref<engine::reflow::FocusEvent> event_) {
    return VBox::onBlur(event_);
}

engine::reflow::EventResponse BoardNode::onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) {
    return VBox::onMouseButtonDown(event_);
}

engine::reflow::EventResponse BoardNode::onMouseButtonUp(cref<engine::reflow::MouseEvent> event_) {
    return VBox::onMouseButtonUp(event_);
}

engine::reflow::EventResponse BoardNode::onMouseMove(cref<engine::reflow::MouseMoveEvent> event_) {
    return VBox::onMouseMove(event_);
}

void BoardNode::addInputSlot(mref<sptr<BoardNodeSlot>> slot_) {

    auto inputs = _inputBox.lock();
    inputs->addChild(_STD move(slot_));
}

void BoardNode::addOutputSlot(mref<sptr<BoardNodeSlot>> slot_) {

    auto outputs = _outputBox.lock();
    outputs->addChild(_STD move(slot_));
}

sptr<BoardNode> BoardNode::make(sptr<hg::editor::gfx::graph::Node> node_) {

    auto widget = sptr<BoardNode>(new BoardNode());

    /**/

    auto header = make_sptr<HBox>(makeHeaderStyle());
    widget->addChild(header);

    auto symbol = make_sptr<Text>(makeHeaderSymbolStyle());
    symbol->setText("F");

    header->addChild(symbol);

    auto titleBox = make_sptr<VBox>(makeHeaderBoxStyle());
    header->addChild(titleBox);

    auto title = make_sptr<Text>(makeHeaderTitleStyle());

    if (node_->getName().empty()) {
        title->setText(string { node_->getNodeTypeName() });
    } else {
        title->setText(node_->getName());
    }

    titleBox->addChild(title);

    auto typeName = make_sptr<Text>(makeHeaderSubtitleStyle());
    if (not node_->getName().empty()) {
        typeName->setText(string { node_->getNodeTypeName() });
    }

    titleBox->addChild(typeName);

    /**/

    auto body = make_sptr<HBox>(makeBodyStyle());
    widget->addChild(body);

    auto inputBox = make_sptr<VBox>(makeBodyTripletStyle());
    auto paramBox = make_sptr<VBox>(makeBodyTripletStyle());
    auto outputBox = make_sptr<VBox>(makeBodyTripletStyle());

    paramBox->style()->width = ReflowUnit { ReflowUnitType::eRelative, 1.F };
    paramBox->style()->reflowShrink = 1.F;

    body->addChild(inputBox);
    body->addChild(paramBox);
    body->addChild(outputBox);

    widget->_inputBox = inputBox;
    widget->_paramBox = paramBox;
    widget->_outputBox = outputBox;

    /**/

    auto limit = node_->countInputSlots();
    for (u64 idx = 0; idx < limit; ++idx) {
        const auto slot = node_->getInputSlot(idx);
        widget->addInputSlot(BoardNodeSlot::makeInput(slot));
    }

    limit = node_->countParameters();
    for (u64 idx = 0; idx < limit; ++idx) {
        const auto param = node_->getParameter(idx);
        // TODO:
    }

    limit = node_->countOutputSlots();
    for (u64 idx = 0; idx < limit; ++idx) {
        const auto slot = node_->getOutputSlot(idx);
        widget->addOutputSlot(BoardNodeSlot::makeOutput(slot));
    }

    /**/

    return widget;
}

/**/

static sptr<BoundStyleSheet> makeBoardNodeStyle() {
    return BoundStyleSheet::make(
        StyleSheet {
            .width = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .height = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .padding = { true, Padding { 6.F } },
            .reflowSpacing = { true, ReflowSpacing::eSpaceAround },
            .borderRadius = { true, Padding { 16.F } },
            .color = { true, color::Dark::backgroundDefault }
        }
    );
}

/**/

sptr<BoundStyleSheet> makeHeaderStyle() {
    return BoundStyleSheet::make(
        StyleSheet {
            .width = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .height = { true, ReflowUnit { ReflowUnitType::eAbsolute, 24.F } },
            .wrap = { true, ReflowWrap::eNoWrap },
            .reflowSpacing = { true, ReflowSpacing::eEnd },
            .reflowShrink = { true, 1.F },
            .reflowGrow = { true, 1.F },
            .color = { true, color::Dark::backgroundDefault },
        }
    );
}

sptr<BoundStyleSheet> makeHeaderSymbolStyle() {
    auto style = BoundStyleSheet::make(
        StyleSheet {
            .minHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .margin = { true, Margin { 4.F } },
            .color = { true, color::Dark::grey },
            .fontSize = { true, 24.F }
        }
    );

    /**/

    style->pushStyle(
        StyleChainEntry {
            Style::TitleKey,
            nullptr,
            Style::get()->getStyleSheet(Style::TitleKey)
        }
    );

    /**/

    return style;
}

sptr<BoundStyleSheet> makeHeaderBoxStyle() {
    return BoundStyleSheet::make(
        StyleSheet {
            .width = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .maxWidth = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .height = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .maxHeight = { true, ReflowUnit { ReflowUnitType::eAuto, 01.F } },
            .wrap = { true, ReflowWrap::eNoWrap },
            .colGap = { true, ReflowUnit { ReflowUnitType::eAbsolute, 2.F } },
            .padding = { true, Padding { 4.F } },
            .reflowSpacing = { true, ReflowSpacing::eSpaceAround },
            .color = { true, color::Dark::backgroundDefault },
        }
    );
}

sptr<BoundStyleSheet> makeHeaderTitleStyle() {
    auto style = BoundStyleSheet::make(
        StyleSheet {
            .color = { true, color::Dark::white },
            .fontSize = { true, 14.F },
            .textAlign = { true, TextAlign::eLeftMiddle },
        }
    );

    /**/

    style->pushStyle(
        StyleChainEntry {
            Style::TitleKey,
            nullptr,
            Style::get()->getStyleSheet(Style::TitleKey)
        }
    );

    /**/

    return style;
}

sptr<BoundStyleSheet> makeHeaderSubtitleStyle() {
    auto style = BoundStyleSheet::make(
        StyleSheet {
            .color = { true, color::Dark::grey },
            .fontSize = { true, 11.F },
            .textAlign = { true, TextAlign::eLeftMiddle },
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

/**/

sptr<BoundStyleSheet> makeBodyStyle() {
    return BoundStyleSheet::make(
        StyleSheet {
            .width = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .maxWidth = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .height = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .maxHeight = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .reflowSpacing = { true, ReflowSpacing::eSpaceBetween },
            .reflowGrow = { true, 1.F },
            .color = { true, color::Dark::backgroundDefault },
        }
    );
}

sptr<BoundStyleSheet> makeBodyTripletStyle() {
    return BoundStyleSheet::make(
        StyleSheet {
            .width = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .maxWidth = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .minHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .height = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .maxHeight = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .colGap = { true, ReflowUnit { ReflowUnitType::eAbsolute, 4.F } },
            .reflowSpacing = { true, ReflowSpacing::eSpaceAround },
            .reflowGrow = { true, 1.F },
            .color = { true, color::Dark::backgroundDefault },
        }
    );
}
