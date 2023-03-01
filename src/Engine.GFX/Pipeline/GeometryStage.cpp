#include "GeometryStage.hpp"

using namespace hg::engine::gfx::pipeline;
using namespace hg::engine::gfx;
using namespace hg;

const ShaderSlot& GeometryStage::shaderSlot() const noexcept {
    return _shaderSlot;
}

ShaderSlot& GeometryStage::shaderSlot() noexcept {
    return _shaderSlot;
}
