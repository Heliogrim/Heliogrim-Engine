#include "RevDepthStage.hpp"

#include "RevDepthSharedNode.hpp"
#include "RevDepthStaticNode.hpp"

using namespace hg::engine::gfx::glow::render;
using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

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
