#include "Panel.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Panel::Panel() = default;

void Panel::render(const ptr<UICommandBuffer> cmd_) {
    ReflowContainer::render(cmd_);
}