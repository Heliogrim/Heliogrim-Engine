#pragma once

#include "DragDropEvent.hpp"
#include "EventReply.hpp"
#include "FocusNavigation.hpp"
#include "KeyboardEvent.hpp"
#include "MouseEvent.hpp"
#include "MouseMoveEvent.hpp"
#include "WheelEvent.hpp"
#include "../Widget/__fwd.hpp"
#include "../Window/__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	class EventRouter {
	public:
		using this_type = EventRouter;

	public:
		EventRouter() noexcept = default;

		virtual ~EventRouter() noexcept = default;

	private:
		struct WindowRoute {
			wptr<Window> window;
			Vector<wptr<Widget>> hover;
		};

		Vector<WindowRoute> _routes;
		struct Press {
			wptr<Widget> widget;
			u32 button;
		};

		Vector<Press> _pressed;

		wptr<Widget> _captured;
		u32 _captureButton = 0uL;

		// Bumped on every focus change, lets a focus change detect a nested one from its blur or focus handlers.
		u64 _focusVersion = 0uLL;

	public:
		void registerWindow(ref<const sptr<Window>> window_);

		void unregisterWindow(nmpt<const Window> window_);

	public:
		EventReply route(ref<const sptr<Window>> window_, ref<const input::event::MouseButtonEvent> event_);

		EventReply route(ref<const sptr<Window>> window_, ref<const input::event::MouseMoveEvent> event_);

		EventReply route(ref<const sptr<Window>> window_, ref<const input::event::MouseWheelEvent> event_);

		EventReply route(ref<const sptr<Window>> window_, ref<const input::event::KeyboardEvent> event_);

		EventReply route(ref<const sptr<Window>> window_, ref<const input::event::DragDropEvent> event_);

	public:
		void focus(ref<const sptr<Window>> window_, ref<const sptr<Widget>> target_);

		void blur(ref<const sptr<Window>> window_);

		[[nodiscard]] FocusSnapshot snapshotFocus(ref<const sptr<Window>> window_) const;

		/**
		 * Focus the deepest widget of the snapshot still mounted in the window, else the window content.
		 */
		void restoreFocus(ref<const sptr<Window>> window_, ref<const FocusSnapshot> snapshot_);

	public:
		/**
		 * Widgets under the point, root first.
		 */
		[[nodiscard]] Vector<sptr<Widget>> hitPath(ref<const sptr<Window>> window_, const math::vec2 point_) const;

		[[nodiscard]] sptr<Widget> captured() const noexcept;

		/**
		 * Window a pointer event outside every window belongs to: the capture's, else the one holding pressed widgets.
		 */
		[[nodiscard]] sptr<Window> pointerOwner();

	protected:
		virtual void onWindowFocusChanged([[maybe_unused]] ref<const sptr<Window>> window_) {}

	protected:
		[[nodiscard]] static bool intersects(
			ref<const math::vec2> off_,
			ref<const math::vec2> size_,
			ref<const math::vec2> point_
		) noexcept;

	private:
		[[nodiscard]] static bool contains(ref<const Widget> widget_, ref<const math::vec2> point_) noexcept;

	private:
		[[nodiscard]] ptr<WindowRoute> findRoute(const ptr<const Window> window_) noexcept;

		[[nodiscard]] sptr<Window> captureWindow(cref<sptr<Window>> window_) const;

		[[nodiscard]] Vector<sptr<Widget>> capturePath(cref<sptr<Window>> window_);

		// Drops the presses of button_, a widget stays pressed while another button still holds it.
		void releasePress(u32 button_);

		void apply(cref<sptr<Window>> window_, ref<const EventReply> reply_, cref<sptr<Widget>> capturer_, u32 button_);
	};
}
