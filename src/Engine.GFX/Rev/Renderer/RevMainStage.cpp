#include "RevMainStage.hpp"

#include "RevMainSharedNode.hpp"
#include "RevMainSkyNode.hpp"
#include "RevMainStaticNode.hpp"

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

void RevMainStage::setup(cref<sptr<Device>> device_) {
    RenderStage::setup(device_);

    /**
     * Instantiate Shared Node
     */
    auto snode { new RevMainSharedNode() };

    /**
     * Populate Multiplexer
     */
    multiplexer().push({ snode });
    multiplexer().push(new RevMainSkyNode(snode));
    multiplexer().push(new RevMainStaticNode(snode));
}

void RevMainStage::destroy() {
    RenderStage::destroy();
}
