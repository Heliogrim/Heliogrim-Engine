#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/ObjectLayer.h>

/**/

#include "jolt.inc.hpp"

namespace hg::engine::physics {
	namespace PhysicsLayer {
		enum PhysicsObjectLayer : JPH::ObjectLayer {
			eStatic = 0x0,
			eMoving = 0x1
		};
	}
}
