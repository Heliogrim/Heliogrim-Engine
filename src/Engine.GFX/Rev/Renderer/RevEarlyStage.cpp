#include "RevEarlyStage.hpp"

#include "RevEarlyBrdfLutNode.hpp"
#include "RevEarlyEnvIrradianceNode.hpp"
#include "RevEarlyEnvPrefilterNode.hpp"
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
    auto brdfLutNode { new RevEarlyBrdfLutNode() };// Pre-computed Brdf Lut
    auto prefilterNode { new RevEarlyEnvPrefilterNode() };// Pre-compute Prefilter Cube
    auto irradNode { new RevEarlyEnvIrradiance() };// Pre-compute Irradiance Cube

    /**
     * Populate Multiplexer
     */
    multiplexer().push({ sfNode });
    multiplexer().push({ brdfLutNode });
    multiplexer().push({ prefilterNode });
    multiplexer().push({ irradNode });
}

void RevEarlyStage::destroy() {
    RenderStage::destroy();
}
