#include "VkModuleCompiler.hpp"

#include <Engine.Accel.Lang/Ir/Diagnostic.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Graphics/Graphics.hpp>
#include <Engine.Graphics/Device/Device.hpp>
#include <Engine.Logging/LogSink.hpp>

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

	if (source->spirv.empty()) {
		engine::diag::LogSink {}.emit(
			lang::ir::Diagnostic::error(lang::codes::asl("internal-spirv-emit"), "module source carries no spir-v words")
		);
		return nullptr;
	}

	/* Compile spirv byte code to vk shader module */

	sptr<graphics::Device> device = Engine::getEngine()->getGraphics()->getCurrentDevice();

	vk::ShaderModuleCreateInfo smci {
		vk::ShaderModuleCreateFlags {}, source->spirv.size() * sizeof(u32), source->spirv.data(),
		nullptr
	};
	vk::ShaderModule vkModule {};
	if (device->vkDevice().createShaderModule(&smci, nullptr, &vkModule) != vk::Result::eSuccess) {
		engine::diag::LogSink {}.emit(
			lang::ir::Diagnostic::error(lang::codes::asl("internal-spirv-emit"), "vulkan rejected the shader module")
		);
		return nullptr;
	}

	/**/

	return make_uptr<VkCompiledModule>(
		reinterpret_cast<_::VkShaderModule>(vkModule.operator VkShaderModule()),
		std::move(source->bindings)
	);
}
