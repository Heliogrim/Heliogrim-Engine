#include "RevDepthStage.hpp"

#include "RevDepthSharedNode.hpp"
#include "RevDepthStaticNode.hpp"

using namespace ember::engine::gfx::glow::render;
using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

void RevDepthStage::setup(cref<sptr<Device>> device_) {
    RenderStage::setup(device_);

    /**
     * Instantiate Shared Node
     */
    auto snode { new RevDepthSharedNode() };

    /**
     * Populate Multiplexer
     */
    multiplexer().push({ snode });
    multiplexer().push(new RevDepthStaticNode(snode));
}

void RevDepthStage::destroy() {
    RenderStage::destroy();
}
