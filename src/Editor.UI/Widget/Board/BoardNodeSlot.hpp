#pragma once
#include <Engine.Reflow/Widget/HBox.hpp>

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>

#include <Editor.GFX.Graphs/Node/__fwd.hpp>

namespace hg::editor::ui {
    class BoardNodeSlot :
        public engine::reflow::HBox {
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
