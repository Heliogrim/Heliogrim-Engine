#include "BoardNode.hpp"

#include "../../Color/Dark.hpp"
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/HorizontalPanel.hpp>
#include <Engine.Reflow/Widget/Text.hpp>

#include <Editor.GFX.Graphs/Node/Node.hpp>
#include "BoardNodeSlot.hpp"
#include "Editor.UI/Theme/Theme.hpp"

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg::editor;
using namespace hg;

BoardNode::BoardNode() :
    VerticalPanel() {
    /**/
    attr.padding.setValue(Padding { 6.F });
    attr.justify.setValue(ReflowSpacing::eSpaceAround);
}

BoardNode::~BoardNode() = default;

string BoardNode::getTag() const noexcept {
    return VerticalPanel::getTag();
}

engine::reflow::EventResponse BoardNode::onFocus(cref<engine::reflow::FocusEvent> event_) {
    return VerticalPanel::onFocus(event_);
}

engine::reflow::EventResponse BoardNode::onBlur(cref<engine::reflow::FocusEvent> event_) {
    return VerticalPanel::onBlur(event_);
}

engine::reflow::EventResponse BoardNode::onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) {
    return VerticalPanel::onMouseButtonDown(event_);
}

engine::reflow::EventResponse BoardNode::onMouseButtonUp(cref<engine::reflow::MouseEvent> event_) {
    return VerticalPanel::onMouseButtonUp(event_);
}

engine::reflow::EventResponse BoardNode::onMouseMove(cref<engine::reflow::MouseMoveEvent> event_) {
    return VerticalPanel::onMouseMove(event_);
}

void BoardNode::addInputSlot(mref<sptr<BoardNodeSlot>> slot_) {

    auto inputs = _inputBox.lock();
    inputs->addChild(std::move(slot_));
}

void BoardNode::addOutputSlot(mref<sptr<BoardNodeSlot>> slot_) {

    auto outputs = _outputBox.lock();
    outputs->addChild(std::move(slot_));
}

cref<math::vec2> BoardNode::getBoardPosition() const noexcept {
    return _position;
}

void BoardNode::setBoardPosition(cref<math::vec2> position_) {
    _position = position_;
}

sptr<BoardNode> BoardNode::make(sptr<hg::editor::gfx::graph::Node> node_) {

    const auto* const theme = Theme::get();

    auto widget = sptr<BoardNode>(new BoardNode());

    /**/

    auto header = make_sptr<HorizontalPanel>();
    header->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 24.F });
    header->attr.justify.setValue(ReflowSpacing::eEnd);
    header->attr.flexShrink.setValue(1.F);
    header->attr.flexGrow.setValue(1.F);

    widget->addChild(header);

    auto symbol = make_sptr<Text>();
    symbol->setText("F");

    theme->applyText(symbol);
    symbol->attr.fontSize.setValue(24.F);

    header->addChild(symbol);

    auto titleBox = make_sptr<VerticalPanel>();
    titleBox->attr.colGap.setValue(2.F);
    titleBox->attr.padding.setValue(Padding { 4.F });
    titleBox->attr.justify.setValue(ReflowSpacing::eSpaceAround);

    header->addChild(titleBox);

    auto title = make_sptr<Text>();
    theme->applyText(title);
    title->attr.fontSize.setValue(14.F);

    if (node_->getName().empty()) {
        title->setText(string { node_->getNodeTypeName() });
    } else {
        title->setText(node_->getName());
    }

    titleBox->addChild(title);

    auto typeName = make_sptr<Text>();
    theme->applyLabel(typeName);
    typeName->attr.fontSize.setValue(11.F);

    if (not node_->getName().empty()) {
        typeName->setText(string { node_->getNodeTypeName() });
    }

    titleBox->addChild(typeName);

    /**/

    auto body = make_sptr<HorizontalPanel>();
    body->attr.minWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    body->attr.justify.setValue(ReflowSpacing::eSpaceBetween);
    body->attr.flexGrow.setValue(1.F);

    widget->addChild(body);

    auto inputBox = make_sptr<VerticalPanel>();
    inputBox->attr.minHeight.setValue({ ReflowUnitType::eRelative, 1.F });
    inputBox->attr.colGap.setValue(4.F);
    inputBox->attr.justify.setValue(ReflowSpacing::eSpaceAround);
    inputBox->attr.flexGrow.setValue(1.F);

    auto paramBox = make_sptr<VerticalPanel>();
    paramBox->attr.minHeight.setValue({ ReflowUnitType::eRelative, 1.F });
    paramBox->attr.colGap.setValue(4.F);
    paramBox->attr.justify.setValue(ReflowSpacing::eSpaceAround);
    paramBox->attr.flexGrow.setValue(1.F);

    auto outputBox = make_sptr<VerticalPanel>();
    outputBox->attr.minHeight.setValue({ ReflowUnitType::eRelative, 1.F });
    outputBox->attr.colGap.setValue(4.F);
    outputBox->attr.justify.setValue(ReflowSpacing::eSpaceAround);
    outputBox->attr.flexGrow.setValue(1.F);

    //paramBox->style()->width = ReflowUnit { ReflowUnitType::eRelative, 1.F };
    //paramBox->style()->reflowShrink = 1.F;

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
