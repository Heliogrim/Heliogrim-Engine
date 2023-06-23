#include "VkCompiledModule.hpp"

#include <Engine.Logging/Logger.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Device/Device.hpp>

using namespace hg::engine::gfx::acc;
using namespace hg;

VkCompiledModule::VkCompiledModule(mref<_::VkShaderModule> shaderModule_) noexcept :
    shaderModule(_STD move(shaderModule_)) {}

VkCompiledModule::~VkCompiledModule() {

    if (not shaderModule) {
        return;
    }

    Logger::warn(
        "Catched still existing vkShaderModule `{:x}` on VkCompiledModule `{:x}`.",
        reinterpret_cast<_STD ptrdiff_t>(shaderModule),
        reinterpret_cast<_STD ptrdiff_t>(_STD addressof(*this))
    );

    Engine::getEngine()->getGraphics()->getCurrentDevice()->vkDevice().destroyShaderModule(
        reinterpret_cast<VkShaderModule>(shaderModule)
    );
    shaderModule = nullptr;
}

VkBindingLocation VkCompiledModule::queryBindLocation(cref<AccelerationStageTransferToken> token_) const noexcept {

    const auto it = _mappedLocations.find(token_);
    if (it != _mappedLocations.end()) {
        return it->second;
    }

    return { -1i32, -1i32 };
}
