#include "VertexStage.hpp"

using namespace hg::engine::gfx::pipeline;
using namespace hg::engine::gfx;
using namespace hg;

const ShaderSlot& VertexStage::shaderSlot() const noexcept {
    return _shaderSlot;
}

ShaderSlot& VertexStage::shaderSlot() noexcept {
    return _shaderSlot;
}
