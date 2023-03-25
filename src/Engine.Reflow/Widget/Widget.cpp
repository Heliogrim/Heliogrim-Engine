#include "Widget.hpp"

using namespace hg::engine::reflow;
using namespace hg;

constexpr static WidgetState defaultWidgetState = WidgetState {
    static_cast<WidgetState::value_type>(WidgetStateFlagBits::eVisible) |
    static_cast<WidgetState::value_type>(WidgetStateFlagBits::ePending) |
    static_cast<WidgetState::value_type>(WidgetStateFlagBits::ePendingInherit) |
    static_cast<WidgetState::value_type>(WidgetStateFlagBits::eShift) |
    static_cast<WidgetState::value_type>(WidgetStateFlagBits::eShiftInherit) |
    static_cast<WidgetState::value_type>(WidgetStateFlagBits::eCapture)
};

Widget::Widget() :
    _state(defaultWidgetState) {}

Widget::~Widget() = default;

cref<WidgetState> Widget::state() const noexcept {
    return _state;
}

ref<WidgetState> Widget::state() noexcept {
    return _state;
}

bool Widget::shouldTick() const noexcept {
    return false;
}

void Widget::tick() {}

EventResponse Widget::onFocus(cref<FocusEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onBlur(cref<FocusEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onMouseButtonDown(cref<MouseEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onMouseButtonUp(cref<MouseEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onMouseMove(cref<MouseMoveEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onMouseEnter(cref<MouseMoveEvent> event_) {

    if (not _state.isHover()) {
        _state |= WidgetStateFlagBits::eHover;
        markAsPending();
    }

    return EventResponse::eUnhandled;
}

EventResponse Widget::onMouseLeave(cref<MouseMoveEvent> event_) {

    if (_state.isHover()) {
        _state.unwrap &= (~static_cast<WidgetState::value_type>(WidgetStateFlagBits::eHover));
        markAsPending();
    }

    return EventResponse::eUnhandled;
}

EventResponse Widget::onWheel(cref<WheelEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onDrag(cref<DragDropEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onDrop(cref<DragDropEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onDragEnter(cref<DragDropEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onDragLeave(cref<DragDropEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onDragOver(cref<DragDropEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onTouchStart(cref<TouchEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onTouchEnd(cref<TouchEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onTouchMove(cref<TouchEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onKeyDown(cref<KeyboardEvent> event_) {
    return EventResponse::eUnhandled;
}

EventResponse Widget::onKeyUp(cref<KeyboardEvent> event_) {
    return EventResponse::eUnhandled;
}

const ptr<const Children> Widget::children() const {
    return nullptr;
}

void Widget::setParent(cref<sptr<Widget>> parent_) {
    _parent = parent_;
}

bool Widget::hasParent() const {
    return not _parent.expired();
}

sptr<Widget> Widget::parent() const {
    return _parent.lock();
}

sptr<Widget> Widget::root() const {
    if (not hasParent()) [[unlikely]]
    {
        return nullptr;
    }

    return parent()->root();
}

float Widget::shrinkFactor() const noexcept {
    return 0.F;
}

float Widget::growFactor() const noexcept {
    return 0.F;
}

ReflowPosition Widget::position() const noexcept {
    return ReflowPosition::eStatic;
}

math::vec2 Widget::outerSize() const noexcept {
    return math::vec2 { 0.F };
}

math::vec2 Widget::innerSize() const noexcept {
    return math::vec2 { 0.F };
}

math::vec2 Widget::screenOffset() const noexcept {
    return math::vec2 { 0.F };
}

bool Widget::willChangeLayout(cref<math::vec2> space_, cref<StyleKeyStack> styleStack_) const noexcept {
    return _state.isProxyPending();
}

void Widget::markAsPending(const bool inherited_, const bool suppress_) {

    /*
    if (_state.pending) {
        return;
    }
     */

    if (inherited_) {
        _state |= WidgetStateFlagBits::ePendingInherit;
        _state |= WidgetStateFlagBits::eShiftInherit;
    } else {
        _state |= WidgetStateFlagBits::ePending;
        _state |= WidgetStateFlagBits::eShift;
    }

    if (suppress_) {
        return;
    }

    if (hasParent()) {
        parent()->markAsPending(true);
    }
}

void Widget::clearPending() {
    _state.unwrap &= ~(
        static_cast<WidgetState::value_type>(WidgetStateFlagBits::ePending) |
        static_cast<WidgetState::value_type>(WidgetStateFlagBits::ePendingInherit)
    );
}

void Widget::clearShiftState() {
    _state.unwrap &= ~(
        static_cast<WidgetState::value_type>(WidgetStateFlagBits::eShift) |
        static_cast<WidgetState::value_type>(WidgetStateFlagBits::eShiftInherit)
    );
}

void Widget::markCaptureState(const bool inherited_) {

    _state |= WidgetStateFlagBits::eCapture;

    if (hasParent()) {
        parent()->markCaptureState(true);
    }
}

void Widget::clearCaptureState() {
    _state.unwrap &= ~(
        static_cast<WidgetState::value_type>(WidgetStateFlagBits::eCapture)
    );
}
