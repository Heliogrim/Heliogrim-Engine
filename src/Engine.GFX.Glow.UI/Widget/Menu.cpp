#include "Menu.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Menu::Menu() :
    Popup() {

    _reflowType = ReflowType::eFlexCol;
    _reflowSpacing = ReflowSpacing::eStart;

    _extent.x = -1.F;
    _extent.y = -1.F;
    _padding = math::vec4 { 4.F };
    _background = color { 46.F, 50.F, 58.F, 255.F };

}

void Menu::render(const ptr<UICommandBuffer> cmd_) {
    Reflow::renderContainer(this, cmd_);
}

bool Menu::onMouseButtonEvent(cref<math::ivec2> pointer_, u32 button_, bool down_, u32 modifier_) {
    const auto result = Popup::onMouseButtonEvent(pointer_, button_, down_, modifier_);

    if (not result) {
        setVisible(false);
    }

    return result;
}
