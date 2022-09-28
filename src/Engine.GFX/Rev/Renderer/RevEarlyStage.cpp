#include "RevEarlyStage.hpp"

#include "RevEarlySFNode.hpp"

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

void RevEarlyStage::setup(cref<sptr<Device>> device_) {
    RenderStage::setup(device_);

    /**
     * Instantiate Nodes
     */
    auto sfNode { new RevEarlySFNode() };// Stream Feedback Node

    /**
     * Populate Multiplexer
     */
    multiplexer().push({ sfNode });
}

void RevEarlyStage::destroy() {
    RenderStage::destroy();
}
