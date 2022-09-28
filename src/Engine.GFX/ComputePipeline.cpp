#include "ComputePipeline.hpp"

using namespace ember::engine::gfx;
using namespace ember;

cref<ShaderSlot> ComputePipeline::shaderSlot() const noexcept {
    return _shaderSlot;
}

ref<ShaderSlot> ComputePipeline::shaderSlot() noexcept {
    return _shaderSlot;
}
