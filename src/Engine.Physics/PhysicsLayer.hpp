#pragma once

#include "jolt.inc.hpp"

/**/

#include <Jolt/Physics/Collision/ObjectLayer.h>

namespace hg::engine::physics {
	namespace PhysicsLayer {
		enum PhysicsObjectLayer : JPH::ObjectLayer {
			eStatic = 0x0,
			eMoving = 0x1
		};
	}
}
