#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/SharedFromThis.hpp>

#include "WidgetState.hpp"
#include "../FlowContext.hpp"
#include "../ReflowPosition.hpp"
#include "../Command/ReflowCommandBuffer.hpp"
#include "../Event/DragDropEvent.hpp"
#include "../Event/EventResponse.hpp"
#include "../Event/MouseEvent.hpp"
#include "../Event/MouseMoveEvent.hpp"
#include "../Event/WheelEvent.hpp"
#include "../Event/FocusEvent.hpp"
#include "../Event/TouchEvent.hpp"
#include "../Event/KeyboardEvent.hpp"
#include "../Style/StyleKeyStack.hpp"

namespace hg::engine::reflow {
    class Children;
}

namespace hg::engine::reflow {
    class Widget :
        public SharedFromThis<Widget> {
    public:
        using this_type = Widget;

    protected:
        Widget();

    public:
        virtual ~Widget();

    protected:
        WidgetState _state;

    public:
        [[nodiscard]] cref<WidgetState> state() const noexcept;

        [[nodiscard]] ref<WidgetState> state() noexcept;

    public:
        [[nodiscard]] virtual string getTag() const noexcept = 0;

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
    public:
        virtual EventResponse onFocus(cref<FocusEvent> event_);

        virtual EventResponse onBlur(cref<FocusEvent> event_);

    public:
        virtual EventResponse onMouseButtonDown(cref<MouseEvent> event_);

        virtual EventResponse onMouseButtonUp(cref<MouseEvent> event_);

        virtual EventResponse onMouseMove(cref<MouseMoveEvent> event_);

        virtual EventResponse onMouseEnter(cref<MouseMoveEvent> event_);

        virtual EventResponse onMouseLeave(cref<MouseMoveEvent> event_);

    public:
        virtual EventResponse onWheel(cref<WheelEvent> event_);

    public:
        virtual EventResponse onDrag(cref<DragDropEvent> event_);

        virtual EventResponse onDrop(cref<DragDropEvent> event_);

        virtual EventResponse onDragEnter(cref<DragDropEvent> event_);

        virtual EventResponse onDragLeave(cref<DragDropEvent> event_);

        virtual EventResponse onDragOver(cref<DragDropEvent> event_);

    public:
        virtual EventResponse onTouchStart(cref<TouchEvent> event_);

        virtual EventResponse onTouchEnd(cref<TouchEvent> event_);

        virtual EventResponse onTouchMove(cref<TouchEvent> event_);

    public:
        virtual EventResponse onKeyDown(cref<KeyboardEvent> event_);

        virtual EventResponse onKeyUp(cref<KeyboardEvent> event_);

        /**
         * Widget Relations
         */
    private:
        wptr<Widget> _parent;

    public:
        [[nodiscard]] virtual const ptr<const Children> children() const;

        void setParent(cref<sptr<Widget>> parent_);

        [[nodiscard]] bool hasParent() const;

        [[nodiscard]] sptr<Widget> parent() const;

        [[nodiscard]] virtual sptr<Widget> root() const;

        /**
         * Layout & Graphics
         */
    public:
        virtual void render(const ptr<ReflowCommandBuffer> cmd_) = 0;

        /**
         * Forward flow control ui elements
         *
         * @param ctx_ The context the flow is related to
         * @param space_ The forwarded available space
         * @param limit_ The forwarded maximum space this widget can occupy
         * @param styleStack_ The key stack to support cascaded styles
         */
        virtual void flow(
            cref<FlowContext> ctx_,
            cref<math::vec2> space_,
            cref<math::vec2> limit_,
            _Inout_ ref<StyleKeyStack> styleStack_
        ) = 0;

        /**
         * Forward offset control ui elements
         *
         * @param ctx_ The context the flow is related to
         * @param offset_ The offset applied to this
         */
        virtual void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) = 0;

        /**
         * Layout Related
         */
    public:
        [[nodiscard]] virtual float shrinkFactor() const noexcept;

        [[nodiscard]] virtual float growFactor() const noexcept;

        [[nodiscard]] virtual ReflowPosition position() const noexcept;

        [[nodiscard]] virtual math::vec2 outerSize() const noexcept;

        [[nodiscard]] virtual math::vec2 innerSize() const noexcept;

        [[nodiscard]] virtual math::vec2 screenOffset() const noexcept;

    public:
        [[nodiscard]] virtual bool willChangeLayout(
            cref<math::vec2> space_,
            cref<StyleKeyStack> styleStack_
        ) const noexcept;

        void markAsPending(const bool inherited_ = false, const bool suppress_ = false);

        void clearPending();

        void clearShiftState();

        void markCaptureState(const bool inherited_ = false);

        void clearCaptureState();
    };
}
