#pragma once
#include <Editor.GFX.Graphs/Node/__fwd.hpp>
#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Widget/HorizontalPanel.hpp>

namespace hg::editor::ui {
    class BoardNodeSlot :
        public engine::reflow::HorizontalPanel {
    public:
        using this_type = BoardNodeSlot;

    protected:
        BoardNodeSlot();

    public:
        ~BoardNodeSlot() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    public:
        engine::reflow::EventResponse onFocus(cref<engine::reflow::FocusEvent> event_) override;

        engine::reflow::EventResponse onBlur(cref<engine::reflow::FocusEvent> event_) override;

    public:
        engine::reflow::EventResponse onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) override;

        engine::reflow::EventResponse onMouseButtonUp(cref<engine::reflow::MouseEvent> event_) override;

        engine::reflow::EventResponse onMouseMove(cref<engine::reflow::MouseMoveEvent> event_) override;

    public:
        static sptr<BoardNodeSlot> makeInput(sptr<::hg::editor::gfx::graph::NodeInput> nodeInput_);

        static sptr<BoardNodeSlot> makeOutput(sptr<::hg::editor::gfx::graph::NodeOutput> nodeOutput_);
    };
}
