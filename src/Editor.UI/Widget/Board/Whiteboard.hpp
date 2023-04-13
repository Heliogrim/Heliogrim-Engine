#pragma once
#include <Engine.Reflow/Widget/Panel.hpp>
#include <Engine.Reflow/Children.hpp>

#include "Board.hpp"
#include "Engine.Reflow/Padding.hpp"
#include "Engine.Reflow/ReflowAlignment.hpp"
#include "Engine.Reflow/ReflowSpacing.hpp"
#include "Engine.Reflow/ReflowUnit.hpp"

namespace hg::editor::ui {
    class Whiteboard :
        public engine::reflow::Panel {
    public:
        using this_type = Whiteboard;

    public:
        Whiteboard();

        ~Whiteboard() override;

    public:
        string getTag() const noexcept override;

    public:
        struct Attributes {
            engine::reflow::Attribute<engine::reflow::ReflowUnit> minWidth;
            engine::reflow::Attribute<engine::reflow::ReflowUnit> width;
            engine::reflow::Attribute<engine::reflow::ReflowUnit> maxWidth;

            engine::reflow::Attribute<engine::reflow::ReflowUnit> minHeight;
            engine::reflow::Attribute<engine::reflow::ReflowUnit> height;
            engine::reflow::Attribute<engine::reflow::ReflowUnit> maxHeight;

            engine::reflow::Attribute<float> flexGrow;
            engine::reflow::Attribute<float> flexShrink;
        } attr;

    public:
        engine::reflow::EventResponse onFocus(cref<engine::reflow::FocusEvent> event_) override;

        engine::reflow::EventResponse onBlur(cref<engine::reflow::FocusEvent> event_) override;

    protected:
        engine::reflow::FixedChildren<2> _children;

    public:
        [[nodiscard]] const non_owning_rptr<const engine::reflow::Children> children() const override;

        [[nodiscard]] sptr<Board> getBoard() const noexcept;

        void setBoard(cref<sptr<Widget>> board_);

        [[nodiscard]] sptr<Widget> getOverlay() const noexcept;

        void setOverlay(cref<sptr<Widget>> overlay_);

    public:
        math::vec2 prefetchDesiredSize(cref<engine::reflow::ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(
            cref<engine::reflow::ReflowPassState> passState_
        ) const override;

        void applyLayout(ref<engine::reflow::ReflowState> state_, mref<engine::reflow::LayoutContext> ctx_) override;

    public:
        [[nodiscard]] bool willChangeLayout(
            cref<math::vec2> space_
        ) const noexcept override;
    };
}
