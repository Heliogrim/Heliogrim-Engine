#include "VkModuleCompiler.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Device/Device.hpp>
#include <Engine.Logging/Logger.hpp>

#include "VkCompiledModule.hpp"
#include "VkModuleSource.hpp"

using namespace hg::engine::accel;
using namespace hg;

VkModuleCompiler::~VkModuleCompiler() noexcept = default;

uptr<CompiledModule> VkModuleCompiler::compile(
	cref<smr<AccelerationPipeline>> targetPass_,
	cref<smr<const EffectSpecification>> specifications_,
	mref<uptr<ModuleSource>> source_
) const {

	auto* const source = static_cast<const ptr<VkModuleSource>>(source_.get());

	/* Assert module source and sanitize */

	if (source->targetStage <= ModuleTargetStage::eUndefined || source->targetStage > ModuleTargetStage::eMesh) {
		return nullptr;
	}

	/* Fetch source code and transpile into spirv */

	::hg::assertd(not source->code.text.empty());
	auto byteCode = _spirvCompiler.compile(*source, source->code.text);

	/**/

	if (byteCode.empty()) {
		IM_CORE_ERROR("Failed to compile spirv module from source code.");
		breakpoint();
		return {};
	}

	/* Compile spirv byte code to vk shader module */

	sptr<gfx::Device> device = Engine::getEngine()->getGraphics()->getCurrentDevice();

	vk::ShaderModuleCreateInfo smci {
		vk::ShaderModuleCreateFlags {}, byteCode.size() * sizeof(SpirvWord), reinterpret_cast<u32*>(byteCode.data()),
		nullptr
	};
	const auto vkModule = device->vkDevice().createShaderModule(smci);

	/**/

	return make_uptr<VkCompiledModule>(
		reinterpret_cast<_::VkShaderModule>(vkModule.operator VkShaderModule()),
		std::move(source->bindings)
	);
}
