#include "ComputePipeline.hpp"

using namespace hg::engine::gfx;
using namespace hg;

cref<ShaderSlot> ComputePipeline::shaderSlot() const noexcept {
    return _shaderSlot;
}

ref<ShaderSlot> ComputePipeline::shaderSlot() noexcept {
    return _shaderSlot;
}
