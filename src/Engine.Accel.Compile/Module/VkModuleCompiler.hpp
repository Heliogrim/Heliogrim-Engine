#pragma once

#include <Engine.Common/Sal.hpp>

#include "ModuleCompiler.hpp"

namespace hg::engine::accel {
	class VkModuleCompiler final :
		public ModuleCompiler {
	public:
		using this_type = VkModuleCompiler;

	public:
		VkModuleCompiler() = default;

		~VkModuleCompiler() noexcept override;

	public:
		[[nodiscard]] _Success_(return != nullptr) uptr<class CompiledModule> compile(
			cref<smr<AccelerationPipeline>> targetPass_,
			cref<smr<const class EffectSpecification>> specifications_,
			mref<uptr<struct ModuleSource>> source_
		) const override;
	};
}
