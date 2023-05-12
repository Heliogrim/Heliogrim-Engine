#include "Panel.hpp"

#include "../Children.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Panel::Panel() :
    Widget() {}

Panel::~Panel() = default;

void Panel::renderPanel(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_, engine::color color_) const {

    const auto passState = state_.getStateOf(this);

    const math::vec2 offset = _layoutState.layoutOffset;
    const math::vec2 size = _layoutState.layoutSize;

    const bool hasBorderRadius = false;

    /**/

    if (not hasBorderRadius || true) {

        const math::vec2 c0 { offset.x, offset.y };
        const math::vec2 c1 { offset.x + size.x, offset.y };
        const math::vec2 c2 { offset.x + size.x, offset.y + size.y };
        const math::vec2 c3 { offset.x, offset.y + size.y };

        cmd_->drawQuad(c0, c1, c2, c3, color_);
    }

}

void Panel::render(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_) {

    for (const auto& child : *children()) {

        if (child->state().isVisible() && not cmd_->scissorCull(
            child->layoutState().layoutOffset,
            child->layoutState().layoutSize
        )) {
            child->render(state_, cmd_);
        }

    }
}
