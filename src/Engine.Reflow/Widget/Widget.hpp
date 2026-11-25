#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Reflow.Theming/ReflowClassList.hpp>
#include <Engine.Reflow.Theming/StyleStates.hpp>
#include <Engine.Reflow.Theming/ThemeProvisioner.hpp>
#include <Engine.Reflow/ReflowPhase.hpp>

#include "WidgetState.hpp"
#include "../ReflowAxis.hpp"
#include "../ReflowEventEmitter.hpp"
#include "../ReflowEventNames.hpp"
#include "../ReflowPosition.hpp"
#include "../ReflowState.hpp"
#include "../Event/DragDropEvent.hpp"
#include "../Event/EventReply.hpp"
#include "../Event/EventResponse.hpp"
#include "../Event/FocusEvent.hpp"
#include "../Event/KeyboardEvent.hpp"
#include "../Event/MouseEvent.hpp"
#include "../Event/MouseMoveEvent.hpp"
#include "../Event/TouchEvent.hpp"
#include "../Event/WheelEvent.hpp"
#include "../Layout/LayoutContext.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Graphics;
import Engine.Reflect;
#endif

namespace hg::engine::reflow {
	class Children;
	class DataWatcher;
	class ReflowCommandBuffer;
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
	private:
		friend class AttributeBatch;

		friend void notifyAttributeChange(nmpt<Widget>, const Flag<ReflowPhase, u8>) noexcept;

	public:
		using this_type = Widget;

		using Aabb2d = ::hg::engine::graphics::Aabb2d;

	protected:
		Widget();

		explicit Widget(mref<ReflowClassList> classList_, SharedPtr<Widget> parent_);

	public:
		virtual ~Widget();

	protected:
		WidgetState _state;
		ReflowPassState _layoutState;

	private:
		u8 _batchDepth = 0;
		Flag<ReflowPhase, u8> _batchPhases {};

	public:
		[[nodiscard]] cref<WidgetState> state() const noexcept;

		[[nodiscard]] ref<WidgetState> state() noexcept;

	public:
		[[nodiscard]] cref<ReflowPassState> getLayoutState() const noexcept;

		[[nodiscard]] ref<ReflowPassState> getLayoutState() noexcept;

	private:
		StyleStates _storedStyleStates {};

	public:
		/**
		 * Focus, hover and press derive from the widget state; disable, drag, selected and invalid are stored.
		 */
		[[nodiscard]] StyleStates styleStates() const noexcept;

		void setStyleState(const InteractiveStateFlagBits state_, const bool value_) noexcept;

		/**
		 * Called after any style state changed; the default does nothing.
		 */
		virtual void onStyleStatesChanged();

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
	protected:
		template <class EventType_>
		using listen_fn_type = ReflowEventEmitter::listener_type<EventType_>;
		using listen_handle_type = ReflowEventEmitter::handle_type;

		ReflowEventEmitter _emitter;

	public:
		/**
		 * Whether click and tab navigation may move the focus onto this widget.
		 */
		[[nodiscard]] virtual bool isFocusable() const noexcept;

		void blur();

		void focus();

		virtual EventReply invokeOnFocus(cref<FocusEvent> event_);

		[[nodiscard]] listen_handle_type onFocus(listen_fn_type<FocusEvent> listenFn_);

		virtual EventReply invokeOnBlur(cref<FocusEvent> event_);

		[[nodiscard]] listen_handle_type onBlur(listen_fn_type<FocusEvent> listenFn_);

	public:
		listen_handle_type onClick(listen_fn_type<MouseEvent> listenFn_);

		bool dropOnClick(listen_handle_type handle_);

		virtual EventReply invokeOnPreviewMouseButtonDown(ref<const MouseEvent> event_);

		virtual EventReply invokeOnMouseButtonDown(ref<const MouseEvent> event_);

		virtual EventReply invokeOnMouseButtonUp(ref<const MouseEvent> event_);

		virtual EventReply invokeOnMouseMove(ref<const MouseMoveEvent> event_);

		virtual EventReply invokeOnMouseEnter(ref<const MouseMoveEvent> event_);

		listen_handle_type onMouseEnter(listen_fn_type<MouseMoveEvent> listenFn_);

		virtual EventReply invokeOnMouseLeave(ref<const MouseMoveEvent> event_);

	public:
		virtual EventReply invokeOnWheel(ref<const WheelEvent> event_);

	public:
		virtual EventReply invokeOnDrag(ref<const DragDropEvent> event_);

		virtual EventReply invokeOnDrop(ref<const DragDropEvent> event_);

		virtual EventReply invokeOnDragEnter(ref<const DragDropEvent> event_);

		virtual EventReply invokeOnDragLeave(ref<const DragDropEvent> event_);

		virtual EventReply invokeOnDragOver(ref<const DragDropEvent> event_);

	public:
		virtual EventReply invokeOnTouchStart(ref<const TouchEvent> event_);

		virtual EventReply invokeOnTouchEnd(ref<const TouchEvent> event_);

		virtual EventReply invokeOnTouchMove(ref<const TouchEvent> event_);

	public:
		virtual EventReply invokeOnPreviewKeyDown(ref<const KeyboardEvent> event_);

		virtual EventReply invokeOnKeyDown(ref<const KeyboardEvent> event_);

		virtual EventReply invokeOnKeyUp(ref<const KeyboardEvent> event_);

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

	private:
		/* Nearest provider per context type, misses as empty entries; cleared for the subtree whenever it moves, re-resolved when the provider died. */
		InlineAutoArray<std::pair<type_id, wptr<Widget>>, 2> _contextCache;

	public:
		/**
		 * Context of the given type this widget provides to its subtree; none by default.
		 */
		[[nodiscard]] virtual ptr<void> providedContext(const type_id contextType_) noexcept;

		/**
		 * Nearest ancestor providing the context type, resolved once and cached until this widget moves.
		 */
		[[nodiscard]] nmpt<Widget> contextProvider(const type_id contextType_) noexcept;

		/**
		 * Context of the nearest provider; defined below.
		 */
		template <class Ctx_>
		[[nodiscard]] Opt<ref<Ctx_>> context();

	protected:
		Opt<ref<DataWatcher>> _watcher;

	public:
		virtual void onAttachDataWatcher(ref<DataWatcher> watcher_);

		virtual void onDetachDataWatcher(ref<DataWatcher> watcher_);

		[[nodiscard]] Opt<ref<DataWatcher>> getDataWatcher() const noexcept;

		bool updateDataWatcherReference(bool invalidate_) noexcept;

		/**
		 * Layout- & Graphics-Rendering
		 */
	public:
		virtual void render(const ptr<ReflowCommandBuffer> cmd_) = 0;

	public:
		virtual void cascadeContextChange(bool invalidate_);

		virtual PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const = 0;

		virtual PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const = 0;

		virtual math::fvec2 computeReferenceSize(ReflowAxis axis_) const = 0;

		virtual void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) = 0;

		virtual void applyLayout(ref<ReflowState> state_) = 0;

		[[nodiscard]] virtual math::fvec2 getShrinkFactor() const noexcept = 0;

		[[nodiscard]] virtual math::fvec2 getGrowFactor() const noexcept = 0;

		[[nodiscard]] virtual ReflowPosition position() const noexcept;

		/**
		 * Whether size and prefetch result are independent of the children, so layout changes below stop at this widget.
		 */
		[[nodiscard]] virtual bool isLayoutBoundary() const noexcept;

	public:
		void markAsPending(const bool layout_, const bool render_);

		void markLayoutPending();

		void markPaintPending();

		[[nodiscard]] bool clearSubtreePending();

		[[nodiscard]] bool clearLayoutPending();

		[[nodiscard]] bool clearRenderPending();

		virtual void enumerateDistinctCapture(
			_In_ const u16 compareVersion_,
			_Inout_ ref<Vector<nmpt<const Widget>>> capture_
		) const noexcept;

		void updateRenderVersion(const u16 version_, const bool inherited_ = false);

		virtual void cascadeRenderVersion(const u16 version_, cref<Aabb2d> aabb_);
	};

	template <class Ctx_>
	Opt<ref<Ctx_>> Widget::context() {
		constexpr auto contextType = refl::ctid<Ctx_>();
		auto provider = contextProvider(contextType);
		const auto value = provider != nullptr ? provider->providedContext(contextType) : nullptr;
		if (value == nullptr) {
			return None;
		}
		return Some(*static_cast<ptr<Ctx_>>(value));
	}
}
