#pragma once

#include <Engine.GFX/API/__vkFwd.hpp>

#include "CompiledModule.hpp"

namespace hg::engine::gfx::acc {
    struct VkCompiledModule :
        public CompiledModule {
        _::VkShaderModule shaderModule;
    };
}
