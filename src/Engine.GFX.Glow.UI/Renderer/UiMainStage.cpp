#include "UiMainStage.hpp"

#include "UiMainStageNode.hpp"

using namespace ember::engine::gfx::glow::ui::render;
using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

void UiMainStage::setup(cref<sptr<Device>> device_) {
    RenderStage::setup(device_);

    /**
     * Populate Multiplexer
     */
    multiplexer().push(new UiMainStageNode());
}

void UiMainStage::destroy() {
    RenderStage::destroy();
}
