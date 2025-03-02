#include "Widget.hpp"

#include "../Children.hpp"

using namespace hg::engine::reflow;
using namespace hg;

constexpr static WidgetState defaultWidgetState = WidgetState {
	static_cast<WidgetState::value_type>(WidgetStateFlagBits::eVisible) |
	static_cast<WidgetState::value_type>(WidgetStateFlagBits::ePending) |
	static_cast<WidgetState::value_type>(WidgetStateFlagBits::ePendingInherit)
};

Widget::Widget() :
	_state(defaultWidgetState),
	_layoutState(),
	_staticClassList(),
	_parent() {}

Widget::Widget(mref<ReflowClassList> classList_, SharedPtr<Widget> parent_) :
	_state(defaultWidgetState),
	_layoutState(),
	_staticClassList(::hg::move(classList_)),
	_parent(::hg::move(parent_)) {
	// TODO: Check whether we need to invoke the provision logic
}

Widget::~Widget() = default;

cref<WidgetState> Widget::state() const noexcept {
	return _state;
}

ref<WidgetState> Widget::state() noexcept {
	return _state;
}

cref<ReflowPassState> Widget::layoutState() const noexcept {
	return _layoutState;
}

ref<ReflowPassState> Widget::layoutState() noexcept {
	return _layoutState;
}

ref<const ReflowClassList> Widget::getStaticClassList() const noexcept {
	return _staticClassList;
}

ref<ReflowClassList> Widget::getStaticClassList() noexcept {
	return _staticClassList;
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

void Widget::setParent(mref<sptr<Widget>> parent_) {
    _parent = std::move(parent_);
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

math::vec2 Widget::computeDesiredSize(cref<ReflowPassState> passState_) const {
    return getDesiredSize();
}

math::vec2 Widget::getDesiredSize() const {
    return _layoutState.prefetchedSize;
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

bool Widget::willChangeLayout(cref<math::vec2> space_) const noexcept {
    return _state.isProxyPending();
}

void Widget::enumerateDistinctCapture(
    const u16 compareVersion_,
    ref<Vector<nmpt<const Widget>>> capture_
) const noexcept {

    if (_state.shouldRenderSelf(compareVersion_)) {
        capture_.emplace_back(this);
        return;
    }

    for (const auto& child : *children()) {
        if (child->state().shouldRender(compareVersion_)) {
            child->enumerateDistinctCapture(compareVersion_, capture_);
        }
    }
}

void Widget::markAsPending(const bool inherited_, const bool suppress_) {

    /*
    if (_state.pending) {
        return;
    }
     */

    if (inherited_) {
        _state |= WidgetStateFlagBits::ePendingInherit;
    } else {
        _state |= WidgetStateFlagBits::ePending;
    }

    if (suppress_) {
        return;
    }

    if (hasParent()) {
        parent()->markAsPending(true);
    }
}

WidgetStateFlag Widget::clearPending() {

    const WidgetStateFlag result { _state };
    _state.unwrap &= ~(
        static_cast<WidgetState::value_type>(WidgetStateFlagBits::ePending) |
        static_cast<WidgetState::value_type>(WidgetStateFlagBits::ePendingInherit)
    );

    return result;
}

void Widget::updateRenderVersion(const u16 version_, const bool inherited_) {

    if (inherited_) {
        _state.setProxyRenderVersion(version_);
    } else {
        _state.setRenderVersion(version_);
    }

    if (hasParent()) {
        parent()->updateRenderVersion(version_, true);
    }

}

void Widget::cascadeRenderVersion(const u16 version_, cref<Aabb2d> aabb_) {

    bool forwarded = false;
    for (const auto& child : *children()) {

        const auto& childLayout = child->layoutState();
        const auto childAabb = Aabb2d { childLayout.layoutOffset, childLayout.layoutOffset + childLayout.layoutSize };

        if (childAabb.contains(aabb_)) {
            child->cascadeRenderVersion(version_, aabb_);
            forwarded = true;
            continue;
        }

        if (childAabb.intersects(aabb_)) {
            child->updateRenderVersion(version_);
        }
    }

    if (not forwarded) {
        updateRenderVersion(version_);
    }
}
