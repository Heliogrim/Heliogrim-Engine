#include "VertexStage.hpp"

using namespace ember::engine::gfx::pipeline;
using namespace ember::engine::gfx;
using namespace ember;

const ShaderSlot& VertexStage::shaderSlot() const noexcept {
    return _shaderSlot;
}

ShaderSlot& VertexStage::shaderSlot() noexcept {
    return _shaderSlot;
}
