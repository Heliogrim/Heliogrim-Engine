#pragma once
#include <Engine.Reflow/Widget/VerticalPanel.hpp>

#include <Engine.Reflow/Children.hpp>

#include <Editor.GFX.Graphs/Node/__fwd.hpp>

namespace hg::editor::ui {
    class BoardNodeSlot;
}

namespace hg::editor::ui {
    class BoardNode :
        public engine::reflow::VerticalPanel {
    public:
        using this_type = BoardNode;

    protected:
        BoardNode();

    public:
        ~BoardNode() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    public:
        engine::reflow::EventResponse onFocus(cref<engine::reflow::FocusEvent> event_) override;

        engine::reflow::EventResponse onBlur(cref<engine::reflow::FocusEvent> event_) override;

    public:
        engine::reflow::EventResponse onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) override;

        engine::reflow::EventResponse onMouseButtonUp(cref<engine::reflow::MouseEvent> event_) override;

        engine::reflow::EventResponse onMouseMove(cref<engine::reflow::MouseMoveEvent> event_) override;

    protected:
        wptr<VerticalPanel> _inputBox;
        wptr<VerticalPanel> _paramBox;
        wptr<VerticalPanel> _outputBox;

    public:
        void addInputSlot(mref<sptr<BoardNodeSlot>> slot_);

        void addOutputSlot(mref<sptr<BoardNodeSlot>> slot_);

    private:
        math::vec2 _position;

    public:
        [[nodiscard]] cref<math::vec2> getBoardPosition() const noexcept;

        void setBoardPosition(cref<math::vec2> position_);

    public:
        static sptr<BoardNode> make(sptr<::hg::editor::gfx::graph::Node> node_);
    };
}
