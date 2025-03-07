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

cref<ReflowPassState> Widget::getLayoutState() const noexcept {
	return _layoutState;
}

ref<ReflowPassState> Widget::getLayoutState() noexcept {
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

EventResponse Widget::invokeOnFocus(cref<FocusEvent> event_) {
	return _emitter.emit(ReflowEventNames::FocusEvent, event_);
}

ReflowEventEmitter::handle_type Widget::onFocus(ReflowEventEmitter::listener_type<FocusEvent> listenFn_) {
	return _emitter.on(ReflowEventNames::FocusEvent, ::hg::move(listenFn_));
}

EventResponse Widget::invokeOnBlur(cref<FocusEvent> event_) {
	return _emitter.emit(ReflowEventNames::BlurEvent, event_);
}

Widget::listen_handle_type Widget::onBlur(listen_fn_type<FocusEvent> listenFn_) {
	return _emitter.on(ReflowEventNames::BlurEvent, ::hg::move(listenFn_));
}

Widget::listen_handle_type Widget::onClick(listen_fn_type<MouseEvent> listenFn_) {
	return _emitter.on(ReflowEventNames::ClickEvent, ::hg::move(listenFn_));
}

bool Widget::dropOnClick(listen_handle_type handle_) {
	return _emitter.off<MouseEvent>(ReflowEventNames::ClickEvent, ::hg::move(handle_));
}

EventResponse Widget::invokeOnMouseButtonDown(ref<const MouseEvent> event_) {
	return _emitter.emit(ReflowEventNames::ClickEvent, event_);
}

EventResponse Widget::invokeOnMouseButtonUp(ref<const MouseEvent> event_) {
	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnMouseMove(ref<const MouseMoveEvent> event_) {
	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnMouseEnter(ref<const MouseMoveEvent> event_) {

	if (not _state.isHover()) {
		_state |= WidgetStateFlagBits::eHover;
		markAsPending();
	}

	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnMouseLeave(ref<const MouseMoveEvent> event_) {

	if (_state.isHover()) {
		_state.unwrap &= (~static_cast<WidgetState::value_type>(WidgetStateFlagBits::eHover));
		markAsPending();
	}

	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnWheel(ref<const WheelEvent> event_) {
	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnDrag(ref<const DragDropEvent> event_) {
	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnDrop(ref<const DragDropEvent> event_) {
	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnDragEnter(ref<const DragDropEvent> event_) {
	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnDragLeave(ref<const DragDropEvent> event_) {
	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnDragOver(ref<const DragDropEvent> event_) {
	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnTouchStart(ref<const TouchEvent> event_) {
	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnTouchEnd(ref<const TouchEvent> event_) {
	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnTouchMove(ref<const TouchEvent> event_) {
	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnKeyDown(ref<const KeyboardEvent> event_) {
	return EventResponse::eUnhandled;
}

EventResponse Widget::invokeOnKeyUp(ref<const KeyboardEvent> event_) {
	return EventResponse::eUnhandled;
}

void Widget::setParent(mref<sptr<Widget>> parent_) {
	_parent = std::move(parent_);

	if (cachedUpdateNearestProvisioner(true) != None) {
		cascadeContextChange(true);
	}
}

void Widget::setParent(cref<sptr<Widget>> parent_) {
	_parent = parent_;

	if (cachedUpdateNearestProvisioner(true) != None) {
		cascadeContextChange(true);
	}
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

Opt<ref<const theming::ThemeProvisioner>> Widget::getEffectiveProvisioner() const noexcept {
	return _effectiveProvisioner;
}

Opt<ref<const theming::ThemeProvisioner>> Widget::cachedUpdateNearestProvisioner(const bool localInvalidate_) noexcept {
	if (not localInvalidate_ && _effectiveProvisioner.has_value()) {
		return _effectiveProvisioner;
	}

	if (not hasParent()) {
		return None;
	}

	/**/

	const auto parentProvisioner = parent()->cachedUpdateNearestProvisioner(false);

	if (parentProvisioner.has_value()) {
		// Warning: Check whether we need the self-restoring behaviour in here !?
		this->_effectiveProvisioner = parentProvisioner;
		return parentProvisioner;
	}

	return None;
}

void Widget::cascadeContextChange(const bool invalidate_) {
	for (const auto& child : *children()) {
		if (child->cachedUpdateNearestProvisioner(invalidate_) != None) {
			child->cascadeContextChange(invalidate_);
		}
	}
}

ReflowPosition Widget::position() const noexcept {
	return ReflowPosition::eStatic;
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

		const auto& childLayout = child->getLayoutState();
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
