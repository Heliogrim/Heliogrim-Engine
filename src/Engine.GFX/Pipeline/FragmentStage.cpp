#include "FragmentStage.hpp"

using namespace hg::engine::gfx::pipeline;
using namespace hg::engine::gfx;
using namespace hg;

const ShaderSlot& FragmentStage::shaderSlot() const noexcept {
    return _shaderSlot;
}

ShaderSlot& FragmentStage::shaderSlot() noexcept {
    return _shaderSlot;
}
