#pragma once

#include <Engine.Common/__macro.hpp>

#include "ModuleSource.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Pipeline;
import Engine.Common;
#endif

namespace hg::engine::accel {
	class macro_novtable ModuleBuilder {
	public:
		using this_type = ModuleBuilder;

	public:
		ModuleBuilder() noexcept = default;

		virtual ~ModuleBuilder() noexcept = default;

	public:
		[[nodiscard]] virtual uptr<ModuleSource> build(
			cref<smr<AccelerationPipeline>> targetPass_,
			cref<smr<const class EffectSpecification>> specifications_,
			cref<smr<class StageDerivat>> stage_,
			cref<uptr<ModuleSource>> previous_
		) const = 0;
	};
}
