#pragma once
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Style/StyleSheet.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow/Children.hpp>

#include "Board.hpp"

namespace hg::editor::ui {
    class Whiteboard :
        public engine::reflow::Widget {
    public:
        using this_type = Whiteboard;

    public:
        Whiteboard();

        ~Whiteboard() override;

    public:
        string getTag() const noexcept override;

    public:
        engine::reflow::EventResponse onFocus(cref<engine::reflow::FocusEvent> event_) override;

        engine::reflow::EventResponse onBlur(cref<engine::reflow::FocusEvent> event_) override;

    protected:
        sptr<engine::reflow::BoundStyleSheet> _style;
        engine::reflow::StyleSheet _computedStyle;

    public:
        [[nodiscard]] cref<sptr<engine::reflow::BoundStyleSheet>> style() const noexcept;

        [[nodiscard]] ref<sptr<engine::reflow::BoundStyleSheet>> style() noexcept;

    protected:
        engine::reflow::Children _children;

    public:
        [[nodiscard]] const non_owning_rptr<const engine::reflow::Children> children() const override;

        [[nodiscard]] sptr<Board> getBoard() const noexcept;

        void setBoard(cref<sptr<Widget>> board_);

        [[nodiscard]] sptr<Widget> getOverlay() const noexcept;

        void setOverlay(cref<sptr<Widget>> overlay_);

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
        engine::reflow::StyleKeyStack::set_type _prevStyleStack;
        math::vec2 _prevSpace;

    public:
        [[nodiscard]] bool willChangeLayout(
            cref<math::vec2> space_,
            cref<engine::reflow::StyleKeyStack> styleStack_
        ) const noexcept override;
    };
}
