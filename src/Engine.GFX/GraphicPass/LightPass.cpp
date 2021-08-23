#include "LightPass.hpp"

using namespace ember::engine::gfx;
using namespace ember;

LightPass::LightPass() :
    GraphicPass(GraphicPassMask::eLightPass) {}

void LightPass::setup() {}

void LightPass::destroy() {}

ptr<ModelPassProcessor> LightPass::processor() noexcept {
    return nullptr;
}
