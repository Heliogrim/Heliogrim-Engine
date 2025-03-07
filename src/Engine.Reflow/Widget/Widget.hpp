#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/SharedFromThis.hpp>
#include <Engine.GFX/Aabb.hpp>
#include <Engine.Reflow.Theming/ThemeProvisioner.hpp>

#include "WidgetState.hpp"
#include "../FlowContext.hpp"
#include "../ReflowAxis.hpp"
#include "../ReflowClassList.hpp"
#include "../ReflowEventEmitter.hpp"
#include "../ReflowEventNames.hpp"
#include "../ReflowPosition.hpp"
#include "../ReflowState.hpp"
#include "../Command/ReflowCommandBuffer.hpp"
#include "../Event/DragDropEvent.hpp"
#include "../Event/EventResponse.hpp"
#include "../Event/FocusEvent.hpp"
#include "../Event/KeyboardEvent.hpp"
#include "../Event/MouseEvent.hpp"
#include "../Event/MouseMoveEvent.hpp"
#include "../Event/TouchEvent.hpp"
#include "../Event/WheelEvent.hpp"
#include "../Layout/LayoutContext.hpp"

namespace hg::engine::reflow {
	class Children;
}

namespace hg::engine::reflow {
	struct PrefetchSizing {
		math::fvec2 minSizing;
		math::fvec2 sizing;
	};

	struct PassPrefetchSizing {
		math::fvec2 minSizing;
		math::fvec2 sizing;
		math::fvec2 maxSizing;
	};

	/**/

	class Widget :
		public SharedFromThis<Widget> {
	public:
		using this_type = Widget;

		using Aabb2d = ::hg::engine::gfx::Aabb2d;

	protected:
		Widget();

		explicit Widget(mref<ReflowClassList> classList_, SharedPtr<Widget> parent_);

	public:
		virtual ~Widget();

	protected:
		WidgetState _state;
		ReflowPassState _layoutState;

	public:
		[[nodiscard]] cref<WidgetState> state() const noexcept;

		[[nodiscard]] ref<WidgetState> state() noexcept;

	public:
		[[nodiscard]] cref<ReflowPassState> getLayoutState() const noexcept;

		[[nodiscard]] ref<ReflowPassState> getLayoutState() noexcept;

	protected:
		ReflowClassList _staticClassList;

	public:
		[[nodiscard]] virtual string getTag() const noexcept = 0;

		[[nodiscard]] ref<const ReflowClassList> getStaticClassList() const noexcept;

		[[nodiscard]] ref<ReflowClassList> getStaticClassList() noexcept;

		/**
		 * Abstract System Clocking
		 */
	public:
		[[nodiscard]] virtual bool shouldTick() const noexcept;

		virtual void tick();

		/**
		 * Events
		 *
		 * @details Adopted Synthetic Events https://reactjs.org/docs/events.html
		 */
	private:
		template <class EventType_>
		using listen_fn_type = ReflowEventEmitter::listener_type<EventType_>;
		using listen_handle_type = ReflowEventEmitter::handle_type;

		ReflowEventEmitter _emitter;

	public:
		virtual EventResponse invokeOnFocus(cref<FocusEvent> event_);

		[[nodiscard]] listen_handle_type onFocus(listen_fn_type<FocusEvent> listenFn_);

		virtual EventResponse invokeOnBlur(cref<FocusEvent> event_);

		[[nodiscard]] listen_handle_type onBlur(listen_fn_type<FocusEvent> listenFn_);

	public:
		[[nodiscard]] listen_handle_type onClick(listen_fn_type<MouseEvent> listenFn_);

		bool dropOnClick(listen_handle_type handle_);

		virtual EventResponse invokeOnMouseButtonDown(ref<const MouseEvent> event_);

		virtual EventResponse invokeOnMouseButtonUp(ref<const MouseEvent> event_);

		virtual EventResponse invokeOnMouseMove(ref<const MouseMoveEvent> event_);

		virtual EventResponse invokeOnMouseEnter(ref<const MouseMoveEvent> event_);

		virtual EventResponse invokeOnMouseLeave(ref<const MouseMoveEvent> event_);

	public:
		virtual EventResponse invokeOnWheel(ref<const WheelEvent> event_);

	public:
		virtual EventResponse invokeOnDrag(ref<const DragDropEvent> event_);

		virtual EventResponse invokeOnDrop(ref<const DragDropEvent> event_);

		virtual EventResponse invokeOnDragEnter(ref<const DragDropEvent> event_);

		virtual EventResponse invokeOnDragLeave(ref<const DragDropEvent> event_);

		virtual EventResponse invokeOnDragOver(ref<const DragDropEvent> event_);

	public:
		virtual EventResponse invokeOnTouchStart(ref<const TouchEvent> event_);

		virtual EventResponse invokeOnTouchEnd(ref<const TouchEvent> event_);

		virtual EventResponse invokeOnTouchMove(ref<const TouchEvent> event_);

	public:
		virtual EventResponse invokeOnKeyDown(ref<const KeyboardEvent> event_);

		virtual EventResponse invokeOnKeyUp(ref<const KeyboardEvent> event_);

		/**
		 * Widget Relations
		 */
	private:
		wptr<Widget> _parent;
		Opt<ref<const theming::ThemeProvisioner>> _effectiveProvisioner;

	public:
		[[nodiscard]] virtual const ptr<const Children> children() const = 0;

		void setParent(mref<sptr<Widget>> parent_);

		void setParent(cref<sptr<Widget>> parent_);

		[[nodiscard]] bool hasParent() const;

		[[nodiscard]] sptr<Widget> parent() const;

		[[nodiscard]] virtual sptr<Widget> root() const;

	public:
		[[nodiscard]] Opt<ref<const theming::ThemeProvisioner>> getEffectiveProvisioner() const noexcept;

		[[nodiscard]] virtual Opt<ref<const theming::ThemeProvisioner>> cachedUpdateNearestProvisioner(
			bool localInvalidate_
		) noexcept;

		/**
		 * Layout- & Graphics-Rendering
		 */
	public:
		virtual void render(const ptr<ReflowCommandBuffer> cmd_) = 0;

	public:
		virtual void cascadeContextChange(bool invalidate_);

		virtual PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const = 0;

		virtual PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const = 0;

		virtual void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) = 0;

		virtual void applyLayout(ref<ReflowState> state_) = 0;

		[[nodiscard]] virtual math::fvec2 getShrinkFactor() const noexcept = 0;

		[[nodiscard]] virtual math::fvec2 getGrowFactor() const noexcept = 0;

		[[nodiscard]] virtual ReflowPosition position() const noexcept;

	public:
		void markAsPending(const bool inherited_ = false, const bool suppress_ = false);

		WidgetStateFlag clearPending();

		virtual void enumerateDistinctCapture(
			_In_ const u16 compareVersion_,
			_Inout_ ref<Vector<nmpt<const Widget>>> capture_
		) const noexcept;

		void updateRenderVersion(const u16 version_, const bool inherited_ = false);

		virtual void cascadeRenderVersion(const u16 version_, cref<Aabb2d> aabb_);
	};
}
