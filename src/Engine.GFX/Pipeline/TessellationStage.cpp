#include "TessellationStage.hpp"

using namespace ember::engine::gfx::pipeline;
using namespace ember::engine::gfx;
using namespace ember;

const ShaderSlot& TessellationStage::ctrlShaderSlot() const noexcept {
    return _ctrlShader;
}

const ShaderSlot& TessellationStage::evalShaderSlot() const noexcept {
    return _evalShader;
}
