#include "Widget.hpp"

#include "NullWidget.hpp"
#include "../Children.hpp"
#include "../DataWatcher.hpp"
#include "../Window/Window.hpp"
#include "../Window/WindowManager.hpp"

using namespace hg::engine::reflow;
using namespace hg;

constexpr static WidgetState defaultWidgetState = WidgetState {
	static_cast<WidgetState::value_type>(WidgetStateFlagBits::eVisible) |
	static_cast<WidgetState::value_type>(WidgetStateFlagBits::eLayoutPending) |
	static_cast<WidgetState::value_type>(WidgetStateFlagBits::eRenderPending)
};

Widget::Widget() :
	_state(defaultWidgetState),
	_layoutState(),
	_staticClassList(),
	_emitter(),
	_parent() {}

Widget::Widget(mref<ReflowClassList> classList_, SharedPtr<Widget> parent_) :
	_state(defaultWidgetState),
	_layoutState(),
	_staticClassList(::hg::move(classList_)),
	_emitter(),
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

void Widget::blur() {
	const auto root = std::static_pointer_cast<Window>(this->root());
	if (root == nullptr)
		return;
	WindowManager::get()->dispatch(root, FocusEvent { root });
}

void Widget::focus() {
	const auto root = std::static_pointer_cast<Window>(this->root());
	if (root == nullptr)
		return;
	WindowManager::get()->dispatch(root, FocusEvent { shared_from_this() });
}

EventReply Widget::invokeOnFocus(cref<FocusEvent> event_) {
	return _emitter.emit(ReflowEventNames::FocusEvent, event_);
}

ReflowEventEmitter::handle_type Widget::onFocus(ReflowEventEmitter::listener_type<FocusEvent> listenFn_) {
	return _emitter.on(ReflowEventNames::FocusEvent, ::hg::move(listenFn_));
}

EventReply Widget::invokeOnBlur(cref<FocusEvent> event_) {
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

EventReply Widget::invokeOnMouseButtonDown(ref<const MouseEvent> event_) {
	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnMouseButtonUp(ref<const MouseEvent> event_) {
	return _emitter.emit(ReflowEventNames::ClickEvent, event_);
}

EventReply Widget::invokeOnMouseMove(ref<const MouseMoveEvent> event_) {
	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnMouseEnter(ref<const MouseMoveEvent> event_) {

	if (not _state.isHover()) {
		_state |= WidgetStateFlagBits::eHover;
		// TODO: Check whether we should fully invalidate
		markAsPending(false, true);
	}

	return _emitter.emit(ReflowEventNames::MouseEnterEvent, event_);
}

Widget::listen_handle_type Widget::onMouseEnter(listen_fn_type<MouseMoveEvent> listenFn_) {
	return _emitter.on<MouseMoveEvent>(ReflowEventNames::MouseEnterEvent, ::hg::move(listenFn_));
}

EventReply Widget::invokeOnMouseLeave(ref<const MouseMoveEvent> event_) {

	if (_state.isHover()) {
		_state.unwrap &= (~static_cast<WidgetState::value_type>(WidgetStateFlagBits::eHover));
		// TODO: Check whether we should fully invalidate
		markAsPending(false, true);
	}

	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnWheel(ref<const WheelEvent> event_) {
	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnDrag(ref<const DragDropEvent> event_) {
	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnDrop(ref<const DragDropEvent> event_) {
	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnDragEnter(ref<const DragDropEvent> event_) {
	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnDragLeave(ref<const DragDropEvent> event_) {
	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnDragOver(ref<const DragDropEvent> event_) {
	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnTouchStart(ref<const TouchEvent> event_) {
	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnTouchEnd(ref<const TouchEvent> event_) {
	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnTouchMove(ref<const TouchEvent> event_) {
	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnKeyDown(ref<const KeyboardEvent> event_) {
	return EventResponse::eUnhandled;
}

EventReply Widget::invokeOnKeyUp(ref<const KeyboardEvent> event_) {
	return EventResponse::eUnhandled;
}

void Widget::setParent(mref<sptr<Widget>> parent_) {

	if (this == NullWidget::instance().get()) {
		return;
	}

	// The new slot may place this widget at the same aabb, which finalize would not detect
	markLayoutPending();

	// Mark the old parent before the swap and the new one after; each walk climbs the current parent chain
	if (const auto prev = _parent.lock()) {
		prev->markLayoutPending();
	}
	_parent = std::move(parent_);
	if (const auto next = _parent.lock()) {
		next->markLayoutPending();
	}

	if (cachedUpdateNearestProvisioner(true) != None) {
		cascadeContextChange(true);
	}
	updateDataWatcherReference(true);
}

void Widget::setParent(cref<sptr<Widget>> parent_) {

	if (this == NullWidget::instance().get()) {
		return;
	}

	// The new slot may place this widget at the same aabb, which finalize would not detect
	markLayoutPending();

	// Mark the old parent before the swap and the new one after; each walk climbs the current parent chain
	if (const auto prev = _parent.lock()) {
		prev->markLayoutPending();
	}
	_parent = parent_;
	if (parent_) {
		parent_->markLayoutPending();
	}

	if (cachedUpdateNearestProvisioner(true) != None) {
		cascadeContextChange(true);
	}
	updateDataWatcherReference(true);
}

bool Widget::hasParent() const {
	return not _parent.expired();
}

sptr<Widget> Widget::parent() const {
	return _parent.lock();
}

sptr<Widget> Widget::root() const {
	if (not hasParent()) [[unlikely]] {
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

void Widget::onAttachDataWatcher([[maybe_unused]] ref<DataWatcher> watcher_) {}

void Widget::onDetachDataWatcher([[maybe_unused]] ref<DataWatcher> watcher_) {}

Opt<ref<DataWatcher>> Widget::getDataWatcher() const noexcept {
	return _watcher;
}

bool Widget::updateDataWatcherReference(const bool invalidate_) noexcept {
	if (not invalidate_ && _watcher.has_value()) {
		return false;
	}

	auto watcher = hasParent() ? parent()->getDataWatcher() : None;
	if (watcher == None && _watcher == None) {
		return false;
	}

	if (_watcher.has_value() && watcher.has_value() && std::addressof(*_watcher) == std::addressof(*watcher)) {
		return false;
	}

	if (not _watcher.has_value() && watcher.has_value()) {
		_watcher = watcher;
		this->onAttachDataWatcher(*_watcher);
	}

	if (_watcher.has_value() && not watcher.has_value()) {
		this->onDetachDataWatcher(*_watcher);
		_watcher = None;
	}

	for (const auto& child : *children()) {
		child->updateDataWatcherReference(invalidate_);
	}
	return true;
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

bool Widget::isLayoutBoundary() const noexcept {
	return false;
}

void Widget::markAsPending(const bool layout_, const bool render_) {
	if (layout_) {
		markLayoutPending();

	} else if (render_) {
		markPaintPending();
	}
}

void Widget::markLayoutPending() {

	// A stale own bit under a clean parent would never reach the engine, so only a layout-pending parent ends the walk early
	if (_state.isLayoutPending()) {
		const auto up = parent();
		if (up == nullptr || up->_state.isLayoutPending()) {
			return;
		}
	}

	_state.set(WidgetStateFlagBits::eLayoutPending);

	// The marked widget's own size may have changed, so its parent always runs; above a boundary only the subtree bit is needed.
	auto leftBoundary = false;
	auto next = parent();
	while (next) {
		if (next->_state.isLayoutPending() || (leftBoundary && next->_state.isSubtreePending())) {
			break;
		}

		next->_state.set(leftBoundary ? WidgetStateFlagBits::eSubtreePending : WidgetStateFlagBits::eLayoutPending);
		if (not leftBoundary) {
			leftBoundary = next->isLayoutBoundary();
		}

		next = next->parent();
	}
}

void Widget::markPaintPending() {

	_state.set(WidgetStateFlagBits::eRenderPending);

	auto next = parent();
	while (next && not next->_state.isSubtreePending() && not next->_state.isLayoutPending()) {
		next->_state.set(WidgetStateFlagBits::eSubtreePending);
		next = next->parent();
	}
}

bool Widget::clearSubtreePending() {
	auto result = _state.isSubtreePending();
	_state.unset(WidgetStateFlagBits::eSubtreePending);
	return result;
}

bool Widget::clearLayoutPending() {
	auto result = _state.isLayoutPending();
	_state.unset(WidgetStateFlagBits::eLayoutPending);
	return result;
}

bool Widget::clearRenderPending() {
	auto result = _state.isRenderPending();
	_state.unset(WidgetStateFlagBits::eRenderPending);
	return result;
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

void Widget::updateRenderVersion(const u16 version_, const bool inherited_) {

	if (inherited_) {
		_state.setProxyRenderVersion(version_);
	} else {
		_state.setRenderVersion(version_);
	}

	// Stop at the first ancestor already carrying this version; it has been propagated further up.
	// Sound because the engine resets every version when the tick wraps, so a match was set within this cycle.
	if (hasParent() && parent()->state().proxyRenderVersion() != version_) {
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
