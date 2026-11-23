#pragma once

#include "../__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Effect;
import Engine.Common;
#endif

namespace hg::engine::accel {
	class StageDerivat :
		public Stage {
	public:
		using this_type = StageDerivat;

	public:
		StageDerivat() noexcept;

		StageDerivat(
			mref<nmpt<Stage>> owner_,
			mref<Vector<smr<Stage>>> supersedes_,
			mref<smr<StageModule>> stageModule_,
			mref<StageFlagBits> flagBits_
		);

		~StageDerivat() override;

	private:
		nmpt<Stage> _owner;
		Vector<smr<Stage>> _supersedes;

	public:
		[[nodiscard]] nmpt<Stage> owner() const noexcept;

		[[nodiscard]] Vector<smr<Stage>> supersedes() const noexcept;

	private:
		smr<StageModule> _stageModule;

	public:
		void setStageFlagBits(mref<StageFlagBits> flags_);

		void setStageModule(mref<smr<StageModule>> mod_);

		[[nodiscard]] smr<StageModule> getStageModule() const noexcept;
	};
}
