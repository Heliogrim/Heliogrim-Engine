#include "VkCompiledModule.hpp"

#include <Engine.Logging/Logger.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Device/Device.hpp>

using namespace hg::engine::accel;
using namespace hg;

VkCompiledModule::VkCompiledModule(mref<_::VkShaderModule> shaderModule_, mref<MappedBindings> bindings_) noexcept :
    bindings(std::move(bindings_)),
    shaderModule(std::move(shaderModule_)) {}

VkCompiledModule::~VkCompiledModule() {

    if (not shaderModule) {
        return;
    }

    Logger::warn(
        "Catched still existing vkShaderModule `{:x}` on VkCompiledModule `{:x}`.",
        reinterpret_cast<std::ptrdiff_t>(shaderModule),
        reinterpret_cast<std::ptrdiff_t>(std::addressof(*this))
    );

    Engine::getEngine()->getGraphics()->getCurrentDevice()->vkDevice().destroyShaderModule(
        reinterpret_cast<VkShaderModule>(shaderModule)
    );
    shaderModule = nullptr;
}
