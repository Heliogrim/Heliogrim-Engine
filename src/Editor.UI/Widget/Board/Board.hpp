#pragma once
#include <Engine.Reflow/Widget/Widget.hpp>

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>

namespace hg::editor::ui {
    class Board :
        public engine::reflow::Widget {
    public:
        using this_type = Board;

    public:
        Board();

        ~Board() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    protected:
        sptr<engine::reflow::BoundStyleSheet> _style;
        engine::reflow::StyleSheet _computedStyle;

    private:
        bool _expMouseHold;

    public:
        engine::reflow::EventResponse onFocus(cref<engine::reflow::FocusEvent> event_) override;

        engine::reflow::EventResponse onBlur(cref<engine::reflow::FocusEvent> event_) override;

    public:
        engine::reflow::EventResponse onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) override;

        engine::reflow::EventResponse onMouseButtonUp(cref<engine::reflow::MouseEvent> event_) override;

        engine::reflow::EventResponse onMouseMove(cref<engine::reflow::MouseMoveEvent> event_) override;

    public:
        engine::reflow::EventResponse onWheel(cref<engine::reflow::WheelEvent> event_) override;

    public:
        engine::reflow::EventResponse onKeyDown(cref<engine::reflow::KeyboardEvent> event_) override;

        engine::reflow::EventResponse onKeyUp(cref<engine::reflow::KeyboardEvent> event_) override;

    protected:
        engine::reflow::Children _children;

    public:
        [[nodiscard]] const ptr<const engine::reflow::Children> children() const override;

        void addChild(cref<sptr<Widget>> child_);

    public:
        void render(const ptr<engine::reflow::ReflowCommandBuffer> cmd_) override;

        void flow(
            cref<engine::reflow::FlowContext> ctx_,
            cref<math::vec2> space_,
            cref<math::vec2> limit_,
            ref<engine::reflow::StyleKeyStack> styleStack_
        ) override;

        void shift(cref<engine::reflow::FlowContext> ctx_, cref<math::vec2> offset_) override;

    protected:
        math::vec2 _innerSize;
        math::vec2 _screenOff;

    public:
        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;

        [[nodiscard]] math::vec2 screenOffset() const noexcept override;

    protected:
        math::vec3 _offCenter;
        math::vec3 _prevOffCenter;

    public:
        [[nodiscard]] math::vec2 offCenter() const noexcept;

        [[nodiscard]] float zoomFactor() const noexcept;

    protected:
        engine::reflow::StyleKeyStack::set_type _prevStyleStack;
        math::vec2 _prevSpace;

    public:
        [[nodiscard]] bool willChangeLayout(
            cref<math::vec2> space_,
            cref<engine::reflow::StyleKeyStack> styleStack_
        ) const noexcept override;
    };
}
