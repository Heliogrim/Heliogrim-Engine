#include "EventRouter.hpp"

#include "FocusEvent.hpp"
#include "../Widget/FocusScope.hpp"
#include "../Widget/Widget.hpp"
#include "../Widget/Layer/Host.hpp"
#include "../Window/LayerBand.hpp"
#include "../Window/Window.hpp"

using namespace hg::engine::reflow;
using namespace hg;

static void markState(ref<const sptr<Widget>> widget_, const WidgetStateFlagBits flag_, const bool value_) {
	if (value_) {
		widget_->state().set(flag_);
	} else {
		widget_->state().unset(flag_);
	}
	widget_->markAsPending(false, true);
}

template <typename Event_, typename Invoke_>
static EventReply bubble(ref<const Vector<sptr<Widget>>> path_, ref<const Event_> event_, Invoke_ invoke_) {
	EventReply reply {};
	for (auto it = path_.rbegin(); it != path_.rend(); ++it) {
		const auto next = std::invoke(invoke_, **it, event_);
		reply = merge(reply, next);
		if (next.isConsumed()) {
			break;
		}
	}
	return reply;
}

static constexpr char escape = 0x1B;

static sptr<Widget> windowContent(ref<const sptr<Window>> window_) {
	return (*window_->children())[1];
}

static sptr<Widget> clickTarget(ref<const sptr<Window>> window_, ref<const Vector<sptr<Widget>>> hit_) {
	for (auto it = hit_.rbegin(); it != hit_.rend(); ++it) {
		if ((*it)->isFocusable()) {
			return *it;
		}
	}

	const auto scope = nearestScope(hit_.back());
	if (scope == nullptr) {
		// The implicit window scope remembers the current target while it is still mounted below the content.
		const auto current = window_->getFocusTarget();
		const auto content = windowContent(window_);
		if (current != nullptr && current != window_ && current != content && current->root() == window_) {
			return current;
		}
		return content;
	}
	auto remembered = scope->remembered();
	return remembered != nullptr ? remembered : scope;
}

static LayerBand anchorBand(ref<const sptr<Window>> window_, ref<const sptr<Widget>> anchor_) {
	for (auto next = anchor_; next != nullptr; next = next->parent()) {
		if (next->parent() != window_) {
			continue;
		}
		for (const auto& layer : window_->layers()) {
			if (layer->host.lock() == next) {
				return layer->band;
			}
		}
		break;
	}
	return LayerBand::eContent;
}

bool EventRouter::intersects(ref<const math::vec2> off_, ref<const math::vec2> size_, ref<const math::vec2> point_) noexcept {
	return point_.x >= off_.x && point_.x < off_.x + size_.x && point_.y >= off_.y && point_.y < off_.y + size_.y;
}

bool EventRouter::contains(ref<const Widget> widget_, ref<const math::vec2> point_) noexcept {
	const auto& layout = widget_.getLayoutState();
	return intersects(layout.layoutOffset, layout.layoutSize, point_);
}

void EventRouter::registerWindow(ref<const sptr<Window>> window_) {
	if (findRoute(window_.get()) == nullptr) {
		_routes.emplace_back(WindowRoute { .window = window_, .hover = {} });
	}
}

void EventRouter::unregisterWindow(nmpt<const Window> window_) {
	const auto remove = std::ranges::remove_if(
		_routes,
		[window_](const auto& entry_) {
			return entry_.window.expired() || entry_.window.lock().get() == window_.get();
		}
	);
	_routes.erase(remove.begin(), remove.end());
}

ptr<EventRouter::WindowRoute> EventRouter::findRoute(const ptr<const Window> window_) noexcept {
	for (auto& entry : _routes) {
		if (entry.window.lock().get() == window_) {
			return &entry;
		}
	}
	return nullptr;
}

Vector<sptr<Widget>> EventRouter::hitPath(ref<const sptr<Window>> window_, const math::vec2 point_) const {

	Vector<sptr<Widget>> path {};
	if (window_ == nullptr || not contains(*window_, point_)) {
		return path;
	}

	path.push_back(window_);

	sptr<Widget> next {};
	const auto layers = window_->layers();
	for (auto it = layers.rbegin(); it != layers.rend() && next == nullptr; ++it) {
		if (*it == nullptr || not isHitTestable((*it)->band)) {
			continue;
		}

		const auto host = (*it)->host.lock();
		if (host != nullptr && host->state().isVisible() && contains(*host, point_)) {
			next = host;
		}
	}

	// The title bar shares the content rectangle, so the content (1) is tested before the title bar (0).
	const auto* const windowChildren = window_->children();
	for (const auto idx : { 1uLL, 0uLL }) {
		if (next != nullptr || windowChildren->size() <= idx) {
			continue;
		}

		const auto& child = (*windowChildren)[idx];
		if (child->state().isVisible() && contains(*child, point_)) {
			next = child;
		}
	}

	while (next != nullptr) {
		path.push_back(next);

		const auto* const children = next->children();
		next = nullptr;
		if (children == nullptr) {
			break;
		}

		for (auto it = children->rbegin(); it != children->rend(); ++it) {
			if ((*it)->state().isVisible() && contains(**it, point_)) {
				next = *it;
				break;
			}
		}
	}

	return path;
}

sptr<Widget> EventRouter::captured() const noexcept {
	return _captured.lock();
}

sptr<Window> EventRouter::pointerOwner() {

	// The capturing widget may have unmounted mid-drag, e.g. with its popup.
	if (const auto target = _captured.lock(); target != nullptr && target->root() == nullptr) {
		_captured.reset();
	}

	if (const auto target = _captured.lock()) {
		return std::static_pointer_cast<Window>(target->root());
	}

	for (const auto& press : _pressed) {
		if (const auto widget = press.widget.lock(); widget != nullptr && widget->root() != nullptr) {
			return std::static_pointer_cast<Window>(widget->root());
		}
	}
	return nullptr;
}

sptr<Window> EventRouter::captureWindow(ref<const sptr<Window>> window_) const {
	const auto target = _captured.lock();
	if (target == nullptr || target->root() == nullptr) {
		return window_;
	}
	return std::static_pointer_cast<Window>(target->root());
}

Vector<sptr<Widget>> EventRouter::capturePath(ref<const sptr<Window>> window_) {

	const auto target = _captured.lock();
	if (target == nullptr) {
		return {};
	}

	const auto root = target->root();
	if (root == nullptr) {
		_captured.reset();
		return {};
	}

	if (root != window_) {
		return {};
	}

	Vector<sptr<Widget>> path {};
	for (auto next = target; next != nullptr; next = next->parent()) {
		path.push_back(next);
	}
	std::ranges::reverse(path);
	return path;
}

void EventRouter::releasePress(const u32 button_) {

	Vector<sptr<Widget>> released {};
	std::erase_if(
		_pressed,
		[&released, button_](const auto& press_) {
			if (press_.button != button_) {
				return false;
			}
			if (auto widget = press_.widget.lock()) {
				released.push_back(::hg::move(widget));
			}
			return true;
		}
	);

	for (const auto& widget : released) {
		if (std::ranges::none_of(_pressed, [&widget](const auto& press_) { return press_.widget.lock() == widget; })) {
			markState(widget, WidgetStateFlagBits::ePress, false);
		}
	}
}

void EventRouter::apply(
	ref<const sptr<Window>> window_,
	ref<const EventReply> reply_,
	ref<const sptr<Widget>> capturer_,
	const u32 button_
) {
	if (reply_.releaseCapture) {
		_captured.reset();
	}

	if (capturer_ != nullptr) {
		_captured = capturer_;
		_captureButton = button_;
	}

	if (const auto target = reply_.focus.lock()) {
		focus(window_, target);
	}
}

EventReply EventRouter::route(ref<const sptr<Window>> requested_, ref<const input::event::MouseButtonEvent> event_) {

	maintainPopups();

	// A held capture receives the pointer whichever window reported it.
	const auto window_ = captureWindow(requested_);

	const auto point = math::vec2 { static_cast<float>(event_._pointer.x), static_cast<float>(event_._pointer.y) };
	if (event_._down && closeOnOutsideDown(window_, point)) {
		_dismissButton = event_._button;
		return EventReply::consumed();
	}

	// The release of a dismissing press must not click whatever lies below the closed popup.
	if (not event_._down && _dismissButton == event_._button) {
		_dismissButton.reset();
		return EventReply::consumed();
	}

	const auto hit = hitPath(window_, point);

	if (event_._down) {

		for (const auto& widget : hit) {
			markState(widget, WidgetStateFlagBits::ePress, true);
			_pressed.emplace_back(Press { .widget = widget, .button = event_._button });
		}

		EventReply reply {};
		sptr<Widget> capturer {};
		const auto visit = [&](ref<const sptr<Widget>> widget_, ref<const EventReply> next_) {
			if (next_.capturePointer && capturer == nullptr) {
				capturer = widget_;
			}
			reply = merge(reply, next_);
			return next_.isConsumed();
		};

		bool consumed = false;
		for (auto it = hit.begin(); it != hit.end() && not consumed; ++it) {
			consumed = visit(*it, (*it)->invokeOnPreviewMouseButtonDown(event_));
		}

		for (auto it = hit.rbegin(); it != hit.rend() && not consumed; ++it) {
			consumed = visit(*it, (*it)->invokeOnMouseButtonDown(event_));
		}

		// A focus requested by the reply wins over the click target.
		if (not hit.empty() && reply.focus.expired()) {
			if (const auto target = clickTarget(window_, hit); target != window_->getFocusTarget()) {
				focus(window_, target);
			}
		}

		apply(window_, reply, capturer, event_._button);
		return reply;
	}

	auto path = capturePath(window_);
	if (path.empty()) {
		path = hit;
	}

	const auto reply = bubble(path, event_, &Widget::invokeOnMouseButtonUp);

	releasePress(event_._button);

	if (event_._button == _captureButton) {
		_captured.reset();
	}

	apply(window_, reply, nullptr, event_._button);
	return reply;
}

EventReply EventRouter::route(ref<const sptr<Window>> requested_, ref<const input::event::MouseMoveEvent> event_) {

	maintainPopups();

	// A held capture receives the pointer whichever window reported it.
	const auto window_ = captureWindow(requested_);

	const auto hit = hitPath(window_, math::vec2 { event_._pointer.x, event_._pointer.y });

	if (auto* const route = findRoute(window_.get())) {

		// Attention: Enter and leave handlers may (un)register windows, which invalidates `route`.
		const auto previous = std::exchange(route->hover, {});
		for (auto it = previous.rbegin(); it != previous.rend(); ++it) {
			const auto widget = it->lock();
			if (widget == nullptr || std::ranges::find(hit, widget) != hit.end()) {
				continue;
			}

			markState(widget, WidgetStateFlagBits::eHover, false);
			widget->invokeOnMouseLeave(event_);
		}

		Vector<wptr<Widget>> hover {};
		for (const auto& widget : hit) {
			if (not widget->state().isHover()) {
				markState(widget, WidgetStateFlagBits::eHover, true);
				widget->invokeOnMouseEnter(event_);
			}
			hover.emplace_back(widget);
		}

		if (auto* const current = findRoute(window_.get())) {
			current->hover = ::hg::move(hover);
		}
	}

	auto path = capturePath(window_);
	if (path.empty()) {
		path = hit;
	}

	const auto reply = bubble(path, event_, &Widget::invokeOnMouseMove);
	apply(window_, reply, nullptr, 0uL);
	return reply;
}

EventReply EventRouter::route(ref<const sptr<Window>> window_, ref<const input::event::MouseWheelEvent> event_) {
	maintainPopups();
	const auto path = hitPath(window_, math::vec2 { static_cast<float>(event_._pointer.x), static_cast<float>(event_._pointer.y) });
	const auto reply = bubble(path, event_, &Widget::invokeOnWheel);
	apply(window_, reply, nullptr, 0uL);
	return reply;
}

EventReply EventRouter::route(ref<const sptr<Window>> window_, ref<const input::event::KeyboardEvent> event_) {

	maintainPopups();

	// Attention: Copy the focus path, handlers may move the focus while we walk it.
	Vector<sptr<Widget>> path {};
	for (const auto& entry : window_->getFocusPath()) {
		if (auto widget = entry.lock()) {
			path.push_back(::hg::move(widget));
		}
	}

	EventReply reply {};
	if (event_._down) {
		for (const auto& widget : path) {
			const auto next = widget->invokeOnPreviewKeyDown(event_);
			reply = merge(reply, next);
			if (next.isConsumed()) {
				apply(window_, reply, nullptr, 0uL);
				return reply;
			}
		}
	}

	reply = merge(reply, bubble(path, event_, event_._down ? &Widget::invokeOnKeyDown : &Widget::invokeOnKeyUp));
	apply(window_, reply, nullptr, 0uL);

	if (event_._down && event_._key == '\t' && not reply.isConsumed()) {
		// SDL_KMOD_SHIFT (left | right shift), Engine.Reflow does not see SDL.
		constexpr u16 shiftModifier = 0x0003u;
		const auto current = window_->getFocusTarget();
		const sptr<Widget> scope = current != nullptr ? nearestScope(current) : nullptr;
		const auto next = nextFocusable(scope != nullptr ? scope : windowContent(window_), current, (event_._modifier & shiftModifier) != 0u);
		if (next != nullptr && next != current) {
			focus(window_, next);
		}
	}

	if (event_._down && event_._key == escape && not reply.isConsumed()) {
		for (auto it = _popups.rbegin(); it != _popups.rend(); ++it) {
			if (it->window.lock() == window_) {
				closePopup(PopupHandle { it->id });
				break;
			}
		}
	}

	return reply;
}

EventReply EventRouter::route(ref<const sptr<Window>> window_, ref<const input::event::DragDropEvent> event_) {
	maintainPopups();
	const auto path = hitPath(window_, math::vec2 { static_cast<float>(event_._pointer.x), static_cast<float>(event_._pointer.y) });
	const auto reply = bubble(path, event_, &Widget::invokeOnDrop);
	apply(window_, reply, nullptr, 0uL);
	return reply;
}

void EventRouter::focus(ref<const sptr<Window>> window_, ref<const sptr<Widget>> target_) {

	// An unmounted target or one in another window would leave a foreign focus chain.
	if (target_ == nullptr || target_->root() != window_) {
		return;
	}

	// Moving the focus to another window means the popup windows lost theirs.
	Vector<u64> lost {};
	for (const auto& popup : _popups) {
		if (popup.window.lock() != window_) {
			lost.push_back(popup.id);
		}
	}
	closePopups(lost);

	onWindowFocusChanged(window_);

	const auto version = ++_focusVersion;
	const FocusEvent event { target_ };

	Vector<sptr<Widget>> chain {};
	for (auto next = target_; next != nullptr; next = next->parent()) {
		chain.push_back(next);
	}
	std::ranges::reverse(chain);

	auto& path = window_->_focus;
	u64 keep = 0uLL;
	while (keep < chain.size() && keep < path.size() && path[keep].lock() == chain[keep]) {
		++keep;
	}

	// Attention: Detach one stale entry per blur, a handler moving the focus again sees a coherent path and wins.
	while (path.size() > keep) {
		const auto widget = path.back().lock();
		path.pop_back();
		if (widget == nullptr) {
			continue;
		}
		markState(widget, WidgetStateFlagBits::eFocus, false);
		widget->invokeOnBlur(event);
		if (version != _focusVersion) {
			return;
		}
	}

	for (auto idx = keep; idx < chain.size(); ++idx) {
		path.emplace_back(chain[idx]);
		markState(chain[idx], WidgetStateFlagBits::eFocus, true);
	}

	for (const auto& widget : chain) {
		if (const auto scope = std::dynamic_pointer_cast<FocusScope>(widget); scope != nullptr && scope != target_) {
			scope->_remembered = target_;
		}
	}

	for (auto idx = chain.size(); idx > keep; --idx) {
		if (chain[idx - 1uLL]->invokeOnFocus(event).isConsumed() || version != _focusVersion) {
			break;
		}
	}
}

void EventRouter::blur(ref<const sptr<Window>> window_) {
	focus(window_, window_);
}

FocusSnapshot EventRouter::snapshotFocus(ref<const sptr<Window>> window_) const {
	return FocusSnapshot { .path = window_->_focus };
}

void EventRouter::restoreFocus(ref<const sptr<Window>> window_, ref<const FocusSnapshot> snapshot_) {
	for (auto it = snapshot_.path.rbegin(); it != snapshot_.path.rend(); ++it) {
		if (const auto widget = it->lock(); widget != nullptr && widget != window_ && widget->root() == window_) {
			focus(window_, widget);
			return;
		}
	}
	focus(window_, windowContent(window_));
}

PopupHandle EventRouter::openPopup(
	ref<const sptr<Window>> window_,
	sptr<Widget> content_,
	ref<const PopupAnchor> anchor_,
	wptr<Widget> anchorWidget_,
	Opt<PopupHandle> parent_,
	std::function<void()> onClosed_
) {
	auto snapshot = snapshotFocus(window_);

	// The content has no layout size before its first pass; maintainPopups re-places it once it has one.
	const auto size = content_->getLayoutState().layoutSize;
	const auto scope = make_sptr<FocusScope>(FocusScopeKind::ePopup);
	scope->setContent(::hg::move(content_));

	const auto host = make_sptr<Host>(scope, placePopup(anchor_, size, window_->getClientSize()), window_->getClientSize());
	discard = window_->requestLayer(std::max(LayerBand::ePopup, anchorBand(window_, anchorWidget_.lock())), host);
	scope->setParent(host);
	host->markAsPending(true, true);

	const auto id = ++_popupIds;
	const bool anchored = not anchorWidget_.expired();
	_popups.emplace_back(
		Popup {
			.id = id,
			// A popup anchored inside another one is its child, so the parent closes it first.
			.parent = parent_.has_value() ? parent_->id : popupIdOf(anchorWidget_.lock()),
			.window = window_,
			.host = host,
			.anchor = anchor_,
			.anchorWidget = ::hg::move(anchorWidget_),
			.anchored = anchored,
			.placed = size.x > 0.F && size.y > 0.F,
			.snapshot = ::hg::move(snapshot),
			.onClosed = ::hg::move(onClosed_)
		}
	);

	const auto first = nextFocusable(scope, nullptr, false);
	focus(window_, first != nullptr ? first : scope);

	return PopupHandle { id };
}

void EventRouter::closePopup(const PopupHandle handle_) {

	Vector<u64> children {};
	for (const auto& popup : _popups) {
		if (popup.parent == handle_.id) {
			children.push_back(popup.id);
		}
	}
	closePopups(children);

	const auto it = std::ranges::find_if(_popups, [&handle_](const auto& popup_) { return popup_.id == handle_.id; });
	if (it == _popups.end()) {
		return;
	}

	auto popup = ::hg::move(*it);
	_popups.erase(it);

	if (const auto window = popup.window.lock()) {
		window->dropLayer(popup.host.get());
		restoreFocus(window, popup.snapshot);
	}

	if (popup.onClosed) {
		popup.onClosed();
	}
}

bool EventRouter::hasOpenPopup(nmpt<const Window> window_) const noexcept {
	return std::ranges::any_of(_popups, [window_](const auto& popup_) { return popup_.window.lock().get() == window_.get(); });
}

bool EventRouter::closePopupOf(ref<const sptr<Widget>> widget_) {

	auto id = popupIdOf(widget_);
	if (id == 0uLL) {
		return false;
	}

	for (auto it = _popups.end(); it != _popups.begin();) {
		--it;
		if (it->id == id && it->parent != 0uLL) {
			id = it->parent;
			it = _popups.end();
		}
	}

	closePopup(PopupHandle { id });
	return true;
}

u64 EventRouter::popupIdOf(sptr<Widget> widget_) const {
	for (; widget_ != nullptr; widget_ = widget_->parent()) {
		for (const auto& popup : _popups) {
			if (popup.host == widget_) {
				return popup.id;
			}
		}
	}
	return 0uLL;
}

void EventRouter::closePopups(ref<const Vector<u64>> ids_) {
	// Top first, a popup closes after the ones opened above it.
	for (auto it = ids_.rbegin(); it != ids_.rend(); ++it) {
		closePopup(PopupHandle { *it });
	}
}

void EventRouter::maintainPopups() {

	Vector<u64> stale {};
	for (auto& popup : _popups) {

		const auto window = popup.window.lock();
		const auto anchor = popup.anchorWidget.lock();
		if (window == nullptr || (popup.anchored && (anchor == nullptr || anchor->root() != window))) {
			stale.push_back(popup.id);
			continue;
		}

		const auto size = popup.host->getLayoutState().layoutSize;
		if (not popup.placed && size.x > 0.F && size.y > 0.F) {
			popup.host->setHostOffset(placePopup(popup.anchor, size, window->getClientSize()));
			popup.host->markAsPending(true, true);
			popup.placed = true;
		}
	}

	closePopups(stale);
}

bool EventRouter::closeOnOutsideDown(ref<const sptr<Window>> window_, const math::vec2 point_) {

	bool inside = false;
	ptr<const Popup> top = nullptr;
	for (const auto& popup : _popups) {
		if (popup.window.lock() == window_) {
			inside = inside || contains(*popup.host, point_);
			top = &popup;
		}
	}

	// A flipped or shifted popup may cover its own anchor, a down inside it belongs to the popup.
	if (top == nullptr || inside) {
		return false;
	}

	// A down on the top anchor toggles it closed, its own handler must not reopen it.
	if (const auto anchor = top->anchorWidget.lock(); anchor != nullptr && contains(*anchor, point_)) {
		closePopup(PopupHandle { top->id });
		return true;
	}

	Vector<u64> outside {};
	for (const auto& popup : _popups) {
		if (popup.window.lock() == window_) {
			outside.push_back(popup.id);
		}
	}
	closePopups(outside);
	return true;
}
