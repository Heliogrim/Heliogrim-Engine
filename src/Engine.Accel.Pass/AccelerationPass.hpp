#pragma once
#include <Engine.Common/__macro.hpp>

#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflect;
#endif

namespace hg::engine::accel {
	class macro_novtable AccelerationPass :
		public InheritBase<AccelerationPass> {
	public:
		using this_type = AccelerationPass;

	public:
		constexpr AccelerationPass() noexcept = default;

		constexpr ~AccelerationPass() noexcept override = default;

	public:
		[[nodiscard]] virtual bool compatible(cref<AccelerationPipeline> pipeline_) const noexcept = 0;
	};
}
