#include "RevLightStage.hpp"

using namespace hg::engine::gfx::glow::render;
using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

void RevLightStage::setup(cref<sptr<Device>> device_) {
    RenderStage::setup(device_);
}

void RevLightStage::destroy() {
    RenderStage::destroy();
}

bool RevLightStage::allocate(const ptr<HORenderPass> renderPass_) {
    return RenderStage::allocate(renderPass_);
}

void RevLightStage::free(const ptr<HORenderPass> renderPass_) {
    RenderStage::free(renderPass_);
}
