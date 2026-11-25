#include "EventRouter.hpp"

#include "FocusEvent.hpp"
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

EventReply EventRouter::route(cref<sptr<Window>> window_, ref<const input::event::MouseButtonEvent> event_) {

	const auto hit = hitPath(window_, math::vec2 { static_cast<float>(event_._pointer.x), static_cast<float>(event_._pointer.y) });

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

		apply(window_, reply, capturer, event_._button);
		return reply;
	}

	auto path = capturePath(window_);
	if (path.empty()) {
		path = hit;
	}

	// Warning: Temporary Solution
	if (not hit.empty() && not hit.back()->state().isFocus()) {
		focus(window_, hit.back());
	}

	const auto reply = bubble(path, event_, &Widget::invokeOnMouseButtonUp);

	releasePress(event_._button);

	if (event_._button == _captureButton) {
		_captured.reset();
	}

	apply(window_, reply, nullptr, event_._button);
	return reply;
}

EventReply EventRouter::route(cref<sptr<Window>> window_, ref<const input::event::MouseMoveEvent> event_) {

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

EventReply EventRouter::route(cref<sptr<Window>> window_, ref<const input::event::MouseWheelEvent> event_) {
	const auto path = hitPath(window_, math::vec2 { static_cast<float>(event_._pointer.x), static_cast<float>(event_._pointer.y) });
	const auto reply = bubble(path, event_, &Widget::invokeOnWheel);
	apply(window_, reply, nullptr, 0uL);
	return reply;
}

EventReply EventRouter::route(cref<sptr<Window>> window_, ref<const input::event::KeyboardEvent> event_) {

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
	return reply;
}

EventReply EventRouter::route(cref<sptr<Window>> window_, ref<const input::event::DragDropEvent> event_) {
	const auto path = hitPath(window_, math::vec2 { static_cast<float>(event_._pointer.x), static_cast<float>(event_._pointer.y) });
	const auto reply = bubble(path, event_, &Widget::invokeOnDrop);
	apply(window_, reply, nullptr, 0uL);
	return reply;
}

void EventRouter::focus(ref<const sptr<Window>> window_, ref<const sptr<Widget>> target_) {

	onWindowFocusChanged(window_);

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

	// Attention: Detach the stale suffix first, blur handlers may move the focus again.
	const Vector<wptr<Widget>> stale { path.begin() + keep, path.end() };
	path.erase(path.begin() + keep, path.end());
	for (auto it = stale.rbegin(); it != stale.rend(); ++it) {
		if (const auto widget = it->lock()) {
			widget->invokeOnBlur(event);
		}
	}

	path.insert(path.end(), chain.begin() + keep, chain.end());

	for (auto idx = chain.size(); idx > keep; --idx) {
		if (chain[idx - 1uLL]->invokeOnFocus(event).isConsumed()) {
			break;
		}
	}
}

void EventRouter::blur(ref<const sptr<Window>> window_) {
	focus(window_, window_);
}
