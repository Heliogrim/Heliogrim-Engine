#include "VkModuleCompiler.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Device/Device.hpp>

#include "VkCompiledModule.hpp"
#include "VkModuleSource.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

VkModuleCompiler::~VkModuleCompiler() noexcept = default;

uptr<CompiledModule> VkModuleCompiler::compile(
    cref<smr<AccelerationPass>> targetPass_,
    cref<class SpecificationStorage> specifications_,
    mref<uptr<ModuleSource>> source_
) const {

    auto* const source = static_cast<const ptr<VkModuleSource>>(source_.get());

    /* Assert module source and sanitize */

    if (source->targetStage <= ModuleTargetStage::eUndefined || source->targetStage > ModuleTargetStage::eMesh) {
        return nullptr;
    }

    /* Fetch source code and transpile into spirv */

    const auto& sourceCode = source->sourceCode;
    auto byteCode = _spirvCompiler.compile(*source, sourceCode);

    /**/

    if (byteCode.empty()) {
        return {};
    }

    /* Compile spirv byte code to vk shader module */

    sptr<Device> device = Engine::getEngine()->getGraphics()->getCurrentDevice();

    vk::ShaderModuleCreateInfo smci {
        vk::ShaderModuleCreateFlags {}, byteCode.size() * sizeof(SpirvWord), reinterpret_cast<u32*>(byteCode.data()),
        nullptr
    };
    const auto vkModule = device->vkDevice().createShaderModule(smci);

    /**/

    return make_uptr<VkCompiledModule>(
        reinterpret_cast<_::VkShaderModule>(vkModule.operator VkShaderModule()),
        _STD move(source->mappedLocations)
    );
}
