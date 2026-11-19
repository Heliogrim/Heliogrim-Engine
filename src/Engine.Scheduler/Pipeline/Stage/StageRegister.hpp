#pragma once

#include <Engine.Common/__macro.hpp>

#include "Stage.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::scheduler {
	class PipelineStage;
}

namespace hg::engine::scheduler {
	class macro_novtable StageRegister {
	public:
		using this_type = StageRegister;

	public:
		virtual ~StageRegister() = default;

	public:
		[[nodiscard]] virtual Opt<ref<const Stage>> registerStage(mref<uptr<PipelineStage>> stage_) = 0;

		[[nodiscard]] virtual Opt<ref<const Stage>> getStage(cref<string> identifier_) const noexcept = 0;

		virtual void removeStage(ref<const Stage> stage_) = 0;
	};
}
