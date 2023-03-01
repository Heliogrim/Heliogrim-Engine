#include "TessellationStage.hpp"

using namespace hg::engine::gfx::pipeline;
using namespace hg::engine::gfx;
using namespace hg;

const ShaderSlot& TessellationStage::ctrlShaderSlot() const noexcept {
    return _ctrlShader;
}

const ShaderSlot& TessellationStage::evalShaderSlot() const noexcept {
    return _evalShader;
}
