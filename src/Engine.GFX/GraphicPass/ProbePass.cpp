#include "ProbePass.hpp"

using namespace ember::engine::gfx;
using namespace ember;

ProbePass::ProbePass() :
    GraphicPass(GraphicPassMask::eProbePass) {}

void ProbePass::setup() {}

void ProbePass::destroy() {}

ptr<ModelPassProcessor> ProbePass::processor() noexcept {
    return nullptr;
}
