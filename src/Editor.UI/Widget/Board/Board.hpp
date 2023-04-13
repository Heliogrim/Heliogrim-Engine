#pragma once
#include <Engine.Reflow/Widget/Widget.hpp>

#include <Engine.Reflow/Children.hpp>

#include "Engine.Reflow/ReflowUnit.hpp"

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

    public:
        struct Attributes {
            engine::reflow::Attribute<engine::reflow::ReflowUnit> minWidth;
            engine::reflow::Attribute<engine::reflow::ReflowUnit> width;
            engine::reflow::Attribute<engine::reflow::ReflowUnit> maxWidth;

            engine::reflow::Attribute<engine::reflow::ReflowUnit> minHeight;
            engine::reflow::Attribute<engine::reflow::ReflowUnit> height;
            engine::reflow::Attribute<engine::reflow::ReflowUnit> maxHeight;
        } attr;

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
        void render(
            cref<engine::reflow::ReflowState> state_,
            const ptr<engine::reflow::ReflowCommandBuffer> cmd_
        ) override;

    public:
        math::vec2 prefetchDesiredSize(cref<engine::reflow::ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(
            cref<engine::reflow::ReflowPassState> passState_
        ) const override;

        void applyLayout(ref<engine::reflow::ReflowState> state_, mref<engine::reflow::LayoutContext> ctx_) override;

    protected:
        math::vec3 _offCenter;
        math::vec3 _prevOffCenter;

    public:
        [[nodiscard]] math::vec2 offCenter() const noexcept;

        [[nodiscard]] float zoomFactor() const noexcept;

    public:
        [[nodiscard]] bool willChangeLayout(
            cref<math::vec2> space_
        ) const noexcept override;
    };
}
