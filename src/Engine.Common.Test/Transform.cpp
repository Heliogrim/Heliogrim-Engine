#include "pch.h"

#include <Engine.Common/Math/Transform.hpp>

using namespace hg;

namespace Common {
	TEST(Transform, setLocationAndRotatorKeepsScale) {
		using namespace hg::math;

		Transform t {};
		t.setScale(Vector3f32 { 2.F });

		auto& ret = t.setLocationAndRotator(Location { 1.F, 2.F, 3.F }, Rotator { 10.F, 20.F, 30.F });

		EXPECT_EQ(std::addressof(ret), std::addressof(t));
		EXPECT_EQ(t.location(), (Location { 1.F, 2.F, 3.F }));
		EXPECT_FLOAT_EQ(t.rotator().pitch(), 10.F);
		EXPECT_FLOAT_EQ(t.rotator().yaw(), 20.F);
		EXPECT_FLOAT_EQ(t.rotator().roll(), 30.F);
		EXPECT_EQ(t.scale(), Vector3f32 { 2.F });
	}
}
