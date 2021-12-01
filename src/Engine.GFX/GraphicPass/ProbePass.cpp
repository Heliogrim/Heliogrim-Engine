#include "ProbePass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::gfx;
using namespace ember;

ProbePass::ProbePass(cref<sptr<Device>> device_, const ptr<Swapchain> swapchain_) :
    GraphicPass(device_, swapchain_, GraphicPassMask::eProbePass) {}

void ProbePass::setup() {

    SCOPED_STOPWATCH

}

void ProbePass::destroy() {}

ptr<ModelPassProcessor> ProbePass::processor() noexcept {
    return nullptr;
}
