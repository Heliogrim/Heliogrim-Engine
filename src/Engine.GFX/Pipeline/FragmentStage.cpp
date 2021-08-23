#include "FragmentStage.hpp"

using namespace ember::engine::gfx::pipeline;
using namespace ember::engine::gfx;
using namespace ember;

const ShaderSlot& FragmentStage::shaderSlot() const noexcept {
    return _shaderSlot;
}

ShaderSlot& FragmentStage::shaderSlot() noexcept {
    return _shaderSlot;
}
