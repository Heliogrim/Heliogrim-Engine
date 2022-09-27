#include "RevFinalStage.hpp"

#include "RevFinalComposeNode.hpp"

using namespace ember::engine::gfx::glow::render;
using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

void RevFinalStage::setup(cref<sptr<Device>> device_) {
    RenderStage::setup(device_);

    /**
     * Populate Multiplexer
     */
    multiplexer().push(new RevFinalComposeNode());
}

void RevFinalStage::destroy() {
    RenderStage::destroy();
}
