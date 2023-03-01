#include "RevFinalStage.hpp"

#include "RevFinalComposeNode.hpp"

using namespace hg::engine::gfx::glow::render;
using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

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
