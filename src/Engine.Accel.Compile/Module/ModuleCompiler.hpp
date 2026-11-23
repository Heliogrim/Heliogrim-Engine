#pragma once

#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>
#include <Engine.Accel.Pipeline/Module/CompiledModule.hpp>
#include <Engine.Common/__macro.hpp>

#include "ModuleSource.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel {
	class macro_novtable ModuleCompiler {
	public:
		using this_type = ModuleCompiler;

	public:
		ModuleCompiler() noexcept = default;

		virtual ~ModuleCompiler() noexcept = default;

	public:
		[[nodiscard]] virtual uptr<CompiledModule> compile(
			cref<smr<AccelerationPipeline>> targetPass_,
			cref<smr<const class EffectSpecification>> specifications_,
			mref<uptr<ModuleSource>> source_
		) const = 0;
	};
}
