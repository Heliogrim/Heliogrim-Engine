#include "LightPass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::gfx;
using namespace ember;

LightPass::LightPass(cref<sptr<Device>> device_, const ptr<Swapchain> swapchain_) :
    GraphicPass(device_, swapchain_, GraphicPassMask::eLightPass) {}

void LightPass::setup() {

    SCOPED_STOPWATCH

}

void LightPass::destroy() {}

ptr<ModelPassProcessor> LightPass::processor() noexcept {
    return nullptr;
}
