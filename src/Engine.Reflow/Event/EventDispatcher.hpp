#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "EventResponse.hpp"
#include "../Widget/__fwd.hpp"
#include "../Window/Window.hpp"

#include "DragDropEvent.hpp"
#include "FocusEvent.hpp"
#include "MouseMoveEvent.hpp"
#include "WheelEvent.hpp"
#include "Engine.Common/Collection/Vector.hpp"
#include "Engine.Reflow/Widget/Widget.hpp"

namespace hg::engine::reflow {
	class EventDispatcher {
	public:
		using this_type = EventDispatcher;

	public:
		constexpr virtual ~EventDispatcher() noexcept = default;

	protected:
		[[nodiscard]] bool intersects(
			cref<math::vec2> off_,
			cref<math::vec2> size_,
			cref<math::vec2> point_
		) const noexcept {

			if (point_.x < off_.x || point_.x >= (off_.x + size_.x)) {
				return false;
			}

			if (point_.y < off_.y || point_.y >= (off_.y + size_.y)) {
				return false;
			}

			return true;
		}

	protected:
		virtual void interceptFocusEvent(cref<sptr<Window>> window_, cref<FocusEvent> event_) = 0;

	public:
		template <typename EventType_>
		EventResponse dispatch(cref<sptr<Window>> window_, cref<EventType_> event_) {
			return EventResponse::eUnhandled;
		}

		EventResponse dispatch(cref<sptr<Window>> window_, cref<KeyboardEvent> event_) {

			// Attention: We need a copy, cause key event could cause blur event ( modify on read )
			const auto path { window_->getFocusPath() };
			EventResponse state { EventResponse::eUnhandled };

			for (
				auto it { path.rbegin() };
				it != path.rend() && state != EventResponse::eConsumed;
				++it
			) {

				if (it->expired()) {
					continue;
				}

				const auto lck { it->lock() };
				if (not lck->state().isFocus()) {
					continue;
				}

				const auto result { event_._down ? lck->invokeOnKeyDown(event_) : lck->invokeOnKeyUp(event_) };
				state = MAX(state, result);
			}

			return state;
		}

		EventResponse dispatch(cref<sptr<Window>> window_, cref<MouseEvent> event_) {

			const math::vec2 point { static_cast<float>(event_._pointer.x), static_cast<float>(event_._pointer.y) };

			Stack<sptr<Widget>> backlog {};
			EventResponse state { EventResponse::eUnhandled };

			backlog.push(window_);

			u64 size { 0uLL };
			while (size != backlog.size()) {

				size = backlog.size();
				const auto& next { backlog.top() };

				const auto* children { next->children() };
				if (not children) {
					continue;
				}

				for (auto it { children->rbegin() }; it != children->rend(); ++it) {

					const auto& child { *it };

					if (child->state().isVisible() && intersects(
						child->getLayoutState().layoutOffset,
						child->getLayoutState().layoutSize,
						point
					)) {
						backlog.push(child);
						break;
					}
				}
			}

			/**/

			if (not backlog.empty()) {
				// Warning: Temporary Solution
				if (not backlog.top()->state().isFocus()) {
					const FocusEvent focusEvent { backlog.top() };
					dispatch(window_, focusEvent);
				}
			}

			while (not backlog.empty() && state != EventResponse::eConsumed) {

				const auto& next { backlog.top() };

				const auto result = (event_._down) ? next->invokeOnMouseButtonDown(event_) : next->invokeOnMouseButtonUp(event_);
				state = MAX(state, result);

				backlog.pop();
			}

			return state;
		}

		EventResponse dispatch(cref<sptr<Window>> window_, cref<MouseMoveEvent> event_) {

			const math::vec2 point { static_cast<float>(event_._pointer.x), static_cast<float>(event_._pointer.y) };

			Vector<sptr<Widget>> backlog {};
			EventResponse state { EventResponse::eUnhandled };

			backlog.push_back(window_);

			while (not backlog.empty() && state != EventResponse::eConsumed) {

				const auto next { backlog.back() };

				const auto contained {
					intersects(next->getLayoutState().layoutOffset, next->getLayoutState().layoutSize, point)
				};
				const auto hovered { next->state().isHover() };

				if (not contained && hovered) {
					next->invokeOnMouseLeave(event_);
				}

				if (contained && not hovered) {
					next->invokeOnMouseEnter(event_);
				}

				const auto result = next->invokeOnMouseMove(event_);

				state = MAX(state, result);
				backlog.pop_back();

				const auto* children { next->children() };
				if (not children) {
					continue;
				}

				for (auto it { children->rbegin() }; it != children->rend(); ++it) {

					const auto& child { *it };

					if (child->state().isHover()) {
						backlog.push_back(child);

					} else if (intersects(child->getLayoutState().layoutOffset, child->getLayoutState().layoutSize, point)) {
						backlog.push_back(child);
					}
				}
			}

			return state;
		}

		EventResponse dispatch(cref<sptr<Window>> window_, cref<DragDropEvent> event_) {

			const math::vec2 point {
				static_cast<float>(event_._pointer.x),
				static_cast<float>(event_._pointer.y)
			};

			Stack<sptr<Widget>> backlog {};
			EventResponse state { EventResponse::eUnhandled };

			backlog.push(window_);

			u64 size { 0uLL };
			while (size != backlog.size()) {

				size = backlog.size();
				const auto& next { backlog.top() };

				const auto* children { next->children() };
				if (not children) {
					continue;
				}

				for (auto it { children->rbegin() }; it != children->rend(); ++it) {

					const auto& child { *it };

					if (intersects(child->getLayoutState().layoutOffset, child->getLayoutState().layoutSize, point)) {
						backlog.push(child);
						break;
					}
				}
			}

			/**/

			while (not backlog.empty() && state != EventResponse::eConsumed) {

				const auto& next { backlog.top() };

				// TODO: Differ between, Drag, Drop, DragOver, DragEnter, DragLeave
				auto result { EventResponse::eUnhandled };
				result = next->invokeOnDrop(event_);
				state = MAX(state, result);

				backlog.pop();
			}

			return state;

		}

		EventResponse dispatch(cref<sptr<Window>> window_, cref<WheelEvent> event_) {

			const math::vec2 point { static_cast<float>(event_._pointer.x), static_cast<float>(event_._pointer.y) };

			Stack<sptr<Widget>> backlog {};
			EventResponse state { EventResponse::eUnhandled };

			backlog.push(window_);

			u64 size { 0uLL };
			while (size != backlog.size()) {

				size = backlog.size();
				const auto& next { backlog.top() };

				const auto* children { next->children() };
				if (not children) {
					continue;
				}

				for (auto it { children->rbegin() }; it != children->rend(); ++it) {

					const auto& child { *it };

					if (intersects(child->getLayoutState().layoutOffset, child->getLayoutState().layoutSize, point)) {
						backlog.push(child);
						break;
					}
				}
			}

			/**/

			while (not backlog.empty() && state != EventResponse::eConsumed) {

				const auto& next { backlog.top() };

				const auto result = next->invokeOnWheel(event_);
				state = MAX(state, result);

				backlog.pop();
			}

			return state;
		}

		EventResponse dispatch(cref<sptr<Window>> window_, cref<FocusEvent> event_) {

			interceptFocusEvent(window_, event_);

			/**/

			Stack<sptr<Widget>> backlog {};
			EventResponse state { EventResponse::eUnhandled };

			backlog.push(event_._target);

			u64 size { 0uLL };
			while (size != backlog.size()) {

				size = backlog.size();
				const auto& next { backlog.top() };

				if (next->hasParent()) {
					backlog.push(next->parent());
				}
			}

			/**/

			auto changeIt { window_->getFocusPath().begin() };
			const auto endIt { window_->getFocusPath().end() };
			while (not backlog.empty() && changeIt != endIt && (*changeIt).lock() == backlog.top()) {
				++changeIt;
				backlog.pop();
			}

			/**/

			if (changeIt != endIt) {

				for (auto it { endIt }; it != changeIt;) {

					if ((--it)->expired()) {
						continue;
					}

					it->lock()->invokeOnBlur(event_);
				}

				window_->_focus.erase(changeIt, endIt);
			}

			/**/

			window_->_focus.reserve(backlog.size());
			const auto persistSize { window_->_focus.size() };

			while (not backlog.empty()) {

				const auto& next { backlog.top() };
				window_->_focus.push_back(next);
				backlog.pop();
			}

			/**/

			const auto persistIt { window_->_focus.begin() + MIN(persistSize - 1, 0) };
			for (auto it { window_->_focus.end() }; it != persistIt && state != EventResponse::eConsumed;) {
				const auto result = (--it)->lock()->invokeOnFocus(event_);
				state = MAX(state, result);
			}

			return state;
		}
	};
}
