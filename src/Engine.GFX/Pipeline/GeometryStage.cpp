#include "GeometryStage.hpp"

using namespace ember::engine::gfx::pipeline;
using namespace ember::engine::gfx;
using namespace ember;

const ShaderSlot& GeometryStage::shaderSlot() const noexcept {
    return _shaderSlot;
}

ShaderSlot& GeometryStage::shaderSlot() noexcept {
    return _shaderSlot;
}
