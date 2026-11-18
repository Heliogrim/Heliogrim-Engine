#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine {
	class Config;
}

namespace hg::engine::scheduler {
	struct LanePolicyConfig {
		u32 sliceMicros = 1000uL;

		/** Interactive gets one worker at least once per this period, regardless of the reserve. */
		u32 interactiveFloorMicros = 8000uL;

		/** Background gets one worker at least once per this period, regardless of the reserve. */
		u32 backgroundFloorMicros = 100000uL;

		/** Workers kept off the lower lanes; 0 = by demand (peak Frame demand over the last frames). */
		u32 reserve = 0uL;

		/** A Frame task waiting longer than this while lower work runs raises the reserve at once (demand mode). */
		u32 frameWaitLimitMicros = 250uL;

		/** A Frame push from a worker also wakes a parked worker. */
		bool wakeOnFramePush = false;

	public:
		[[nodiscard]] static LanePolicyConfig fromConfig(ref<const ::hg::engine::Config> config_);
	};
}
