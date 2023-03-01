#include "RevMainStage.hpp"

#include "RevMainSharedNode.hpp"
#include "RevMainSkyNode.hpp"
#include "RevMainStaticNode.hpp"

using namespace hg::engine::gfx::glow::render;
using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

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
