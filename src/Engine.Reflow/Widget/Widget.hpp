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
#include "../Layout/LayoutContext.hpp"
#include "../ReflowState.hpp"
#include "../Attribute/Attribute.hpp"
#include "../Attribute/DynamicAttribute.hpp"

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
        [[nodiscard]] virtual const ptr<const Children> children() const = 0;

        void setParent(cref<sptr<Widget>> parent_);

        [[nodiscard]] bool hasParent() const;

        [[nodiscard]] sptr<Widget> parent() const;

        [[nodiscard]] virtual sptr<Widget> root() const;

        /**
         * Layout & Graphics
         */
    public:
        virtual void render(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_) = 0;

    public:
        virtual math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const = 0;

        virtual math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const;

        virtual void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) = 0;

        /**
         * Layout Related
         */
    public:
        [[nodiscard]] math::vec2 getDesiredSize() const;

    public:
        [[nodiscard]] virtual float shrinkFactor() const noexcept;

        [[nodiscard]] virtual float growFactor() const noexcept;

        [[nodiscard]] virtual ReflowPosition position() const noexcept;

    public:
        [[nodiscard]] virtual bool willChangeLayout(
            cref<math::vec2> space_
        ) const noexcept;

        void markAsPending(const bool inherited_ = false, const bool suppress_ = false);

        void clearPending();

        void clearShiftState();

        void markCaptureState(const bool inherited_ = false);

        void clearCaptureState();
    };
}
