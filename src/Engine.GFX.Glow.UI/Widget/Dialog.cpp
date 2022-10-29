#include "Dialog.hpp"

#include "Editor.UI/Color/Dark.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Dialog::Dialog() :
    Popup() {

    _reflowType = ReflowType::eFlexCol;
    _reflowSpacing = ReflowSpacing::eStart;

    _extent.x = -1.F;
    _extent.y = -1.F;

    _padding = math::vec4 { 8.F };
    _color = editor::ui::color::Dark::backgroundDefault;
}

void Dialog::render(const ptr<UICommandBuffer> cmd_) {

    /**
     *
     */
    const auto& tf { _transform };

    const math::fExtent2D scissor { tf };
    cmd_->pushScissor(scissor);

    /**/
    Reflow::renderContainer(this, cmd_);

    /**
     *
     */
    cmd_->popScissor();
}
