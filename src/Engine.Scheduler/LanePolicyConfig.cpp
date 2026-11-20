#include "LanePolicyConfig.hpp"

#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>

using namespace hg::engine::scheduler;
using namespace hg;

LanePolicyConfig LanePolicyConfig::fromConfig(ref<const engine::Config> config_) {

	constexpr auto fallback = LanePolicyConfig {};
	const auto laneValue = [&config_](const engine::cfg::SchedulerConfigProperty key_, const u32 default_) {
		return config_.getOrDefault<u32>(key_, default_).value_or(default_);
	};

	return LanePolicyConfig {
		.sliceMicros = laneValue(engine::cfg::SchedulerConfigProperty::eLaneSlice, fallback.sliceMicros),
		.interactiveFloorMicros = laneValue(engine::cfg::SchedulerConfigProperty::eInteractiveFloor, fallback.interactiveFloorMicros),
		.backgroundFloorMicros = laneValue(engine::cfg::SchedulerConfigProperty::eBackgroundFloor, fallback.backgroundFloorMicros),
		.reserve = laneValue(engine::cfg::SchedulerConfigProperty::eLaneReserve, fallback.reserve),
		.frameWaitLimitMicros = laneValue(engine::cfg::SchedulerConfigProperty::eFrameWaitLimit, fallback.frameWaitLimitMicros)
	};
}
