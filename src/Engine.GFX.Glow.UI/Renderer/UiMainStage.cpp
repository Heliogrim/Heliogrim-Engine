#include "UiMainStage.hpp"

#include "UiMainStageNode.hpp"

using namespace hg::engine::gfx::glow::ui::render;
using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

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
