#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.GFX/Color/Color.hpp>
#include <Engine.Input/MouseButtonEvent.hpp>
#include <Engine.Input/DragDropEvent.hpp>

#include "../Reflow.hpp"
#include "../Command/UICommandBuffer.hpp"

namespace ember::engine::gfx::glow::ui {

    using WidgetEventEmitter = GlobalEventEmitter;

    class __declspec(novtable) WidgetDelegate {
    public:
        friend class ::ember::engine::gfx::glow::ui::Widget;

    public:
        using this_type = WidgetDelegate;

    private:
        WidgetDelegate() noexcept = default;

    public:
        virtual bool onMouseButtonEvent(cref<math::ivec2> pointer_, u32 button_, bool down_, u32 modifier_) = 0;

        virtual bool onMouseMotionEvent(cref<math::ivec2> pointer_, cref<math::ivec2> delta_, u32 button_,
            u32 modifier_) = 0;

        virtual bool onMouseDragEvent(cref<math::ivec2> pointer_, cref<math::ivec2> delta_, u32 button_,
            u32 modifier_) = 0;

        virtual bool onMouseEnterEvent(cref<math::ivec2> pointer_, bool enter_) = 0;

    public:
        virtual bool onDragDropEvent(cref<input::event::DragDropEvent> event_) = 0;

    public:
        virtual bool onScrollEvent(cref<math::ivec2> pointer_, cref<math::vec2> value_) = 0;

    public:
        virtual bool onFocusEvent(bool focus_) = 0;

    public:
        virtual bool onKeyboardEvent(u32 key_, u32 action_, u32 modifier_) = 0;

        virtual bool onKeyboardCharEvent(u32 code_) = 0;
    };

    struct WidgetBaseState {
        bool focused;
        bool hover;
        bool visible = true;

        /**/

        bool touched = false;
        bool deleted = false;
    };

    class __declspec(novtable) Widget :
        public WidgetDelegate {
    public:
        // ReSharper disable once CppRedundantQualifier
        friend ::ember::engine::gfx::glow::ui::Reflow;

    public:
        using this_type = Widget;

    protected:
        Widget();

        virtual ~Widget() = default;

    public:
        virtual void render(const ptr<UICommandBuffer> cmd_) = 0;

        /**
         * Forward flow control ui elements
         *
         * @param ctx_ The context the flow is related to
         * @param available_ The forwarded available space
         *
         * @returns The used or minimal required space if return dimensions are bigger than available
         */
        virtual math::vec2 flow(cref<UIContext> ctx_, cref<math::vec2> available_);

        /**
         * Bi-directional reflow control ui elements
         *
         * @param available_ The forwarded available space
         * @param reflow_ The flooded target space used to reflow tree
         *
         * @returns The actual consumed space
         */
        virtual math::vec2 reflow(cref<math::vec2> available_, cref<math::vec2> reflow_) {
            return math::vec2 { 0.F };
        };

        /**
         * Forward offset control ui elements
         *
         * @param ctx_ The context the flow is related to
         * @param offset_ The offset applied to this
         */
        virtual void shift(cref<UIContext> ctx_, cref<math::vec2> offset_);

    protected:
        non_owning_rptr<Widget> _parent;

    public:
        [[nodiscard]] const non_owning_rptr<const Widget> root() const noexcept;

        [[nodiscard]] const non_owning_rptr<Widget> root() noexcept;

        [[nodiscard]] non_owning_rptr<Widget> parent() const noexcept;

        void setParent(const non_owning_rptr<Widget> parent_);

    protected:
        Vector<sptr<Widget>> _nodes;

    public:
        [[nodiscard]] ref<Vector<sptr<Widget>>> nodes() noexcept;

        void add(mref<ptr<Widget>> widget_);

        void add(cref<sptr<Widget>> widget_);

        [[nodiscard]] bool has(const ptr<Widget> widget_);

        [[nodiscard]] bool has(cref<sptr<Widget>> widget_);

        void remove(const ptr<Widget> widget_);

        void remove(cref<sptr<Widget>> widget_);

    protected:
        WidgetBaseState _state;

    public:
        [[nodiscard]] bool visible() const noexcept;

        void setVisible(bool visible_);

        [[nodiscard]] bool recursiveVisible() const noexcept;

    public:
        [[nodiscard]] bool focused() const noexcept;

        void requestFocus();

    protected:
        void updateFocus(ptr<Widget> widget_);

    public:
        [[nodiscard]] bool hovered() const noexcept;

    public:
        [[nodiscard]] bool markedAsDeleted() const noexcept;

        void markAsDeleted() noexcept;

        [[nodiscard]] bool markedAsTouched() const noexcept;

        void markAsTouched() noexcept;

    protected:
        math::fExtent2D _transform;

    public:
        [[nodiscard]] cref<math::fExtent2D> transform() const noexcept;

        [[nodiscard]] bool contains(cref<math::ivec2> point_) const noexcept;

        #pragma region Runtime States

    protected:
        math::vec2 _overflowSize;

        #pragma endregion

        //protected:
    public:
        ReflowPosition _reflowPosition;

        //protected:
    public:
        math::vec2 _extent;
        math::vec2 _minExtent;
        math::vec2 _maxExtent;

        //protected:
    public:
        math::vec4 _margin;
        math::vec4 _padding;

        //protected:
    public:
        math::vec4 _borderRadius;

        //protected:
    public:
        engine::color _color;

    public:
        [[nodiscard]] virtual engine::color statedColor() const noexcept;

    public:
        // Warning: experimental
        float _reflowBase = 0.F;
        float _reflowShrink = 0.F;
        float _reflowGrow = 0.F;

        math::vec2 _reflowGapping;

        #pragma region Reflow (Flex) Behaviour

    protected:
        ReflowType _reflowType;

    public:
        [[nodiscard]] ReflowType reflowType() const noexcept;

        void setReflowType(const ReflowType type_);

    protected:
        ReflowSpacing _reflowSpacing;

    public:
        [[nodiscard]] ReflowSpacing reflowSpacing() const noexcept;

        void setReflowSpacing(const ReflowSpacing spacing_);

    protected:
        ReflowWrapping _reflowWrapping;

    public:
        [[nodiscard]] ReflowWrapping reflowWrapping() const noexcept;

        void setReflowWrapping(const ReflowWrapping wrapping_);

    protected:
        ReflowOverflow _reflowOverflow;

    public:
        [[nodiscard]] ReflowOverflow reflowOverflow() const noexcept;

        void setReflowOverflow(const ReflowOverflow overflow_);

        #pragma endregion

        #pragma region Event Behaviour
    protected:
        WidgetEventEmitter _emitter;

    public:
        cref<WidgetEventEmitter> emitter() const noexcept;

        ref<WidgetEventEmitter> emitter() noexcept;

    public:
        using OnClickListener = ::ember::StatelessEventEmitter<input::event::MouseButtonEvent>::function_type;

        bool onMouseButtonEvent(cref<math::ivec2> pointer_, u32 button_, bool down_, u32 modifier_) override;

        bool onMouseMotionEvent(cref<math::ivec2> pointer_, cref<math::ivec2> delta_, u32 button_,
            u32 modifier_) override;

        bool onMouseDragEvent(cref<math::ivec2> pointer_, cref<math::ivec2> delta_, u32 button_,
            u32 modifier_) override;

        bool onMouseEnterEvent(cref<math::ivec2> pointer_, bool enter_) override;

        bool onDragDropEvent(cref<input::event::DragDropEvent> event_) override;

        bool onScrollEvent(cref<math::ivec2> pointer_, cref<math::vec2> value_) override;

        bool onFocusEvent(bool focus_) override;

        bool onKeyboardEvent(u32 key_, u32 action_, u32 modifier_) override;

        bool onKeyboardCharEvent(u32 code_) override;
        #pragma endregion
    };

    class __declspec(novtable) StatefulWidget : public Widget {
    public:
        using this_type = Widget;

    protected:
        StatefulWidget();

        ~StatefulWidget() override = default;
    };

}
