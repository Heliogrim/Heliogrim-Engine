#include "VkCompiledModule.hpp"

#include <Engine.Logging/Logger.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Device/Device.hpp>

using namespace hg::engine::accel;
using namespace hg;

VkCompiledModule::VkCompiledModule(mref<_::VkShaderModule> shaderModule_, mref<MappedBindings> bindings_) noexcept :
    bindings(_STD move(bindings)),
    shaderModule(_STD move(shaderModule)) {}

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
