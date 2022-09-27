#include "RevProbeStage.hpp"

using namespace ember::engine::gfx::glow::render;
using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

void RevProbeStage::setup(cref<sptr<Device>> device_) {
    RenderStage::setup(device_);
}

void RevProbeStage::destroy() {
    RenderStage::destroy();
}

bool RevProbeStage::allocate(const ptr<HORenderPass> renderPass_) {
    return RenderStage::allocate(renderPass_);
}

void RevProbeStage::free(const ptr<HORenderPass> renderPass_) {
    RenderStage::free(renderPass_);
}
