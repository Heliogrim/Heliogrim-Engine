#pragma once

#include <Jolt/Jolt.h>

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

/**/

namespace hg::math {
	class Location;
	class Rotator;
}

template <std::same_as<::hg::math::Location> Location_>
[[nodiscard]] constexpr inline Location_ into(JPH::Vec3 vec3_) {
	return Location_ { vec3_[0], vec3_[1], vec3_[2] };
}

template <std::same_as<::hg::math::Rotator> Rotator_>
[[nodiscard]] constexpr inline Rotator_ into(JPH::Quat quat_) {
	return Rotator_ { ::hg::math::quaternion { quat_.GetX(), quat_.GetY(), quat_.GetZ(), quat_.GetW() } };
}
