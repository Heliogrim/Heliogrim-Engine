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

	TEST(Transform, asMatrixAgreesWithRotatorForCombinedAxes) {
		using namespace hg::math;

		// Both paths describe the very same orientation: Rotator::rotated (closed form) and Transform::asMatrix (quaternion).
		const auto rot = Rotator { 0.3F, 0.7F, -0.2F };
		const auto t = Transform { Location { 1.F, 2.F, 3.F }, Rotator { rot }, Vector3f32 { 1.F } };
		const auto vec = fvec3 { 1.F, 2.F, 3.F };

		const auto expected = rot.rotated(vec) + fvec3 { 1.F, 2.F, 3.F };
		const auto actual = t.asMatrix() * fvec4 { vec.x, vec.y, vec.z, 1.F };

		EXPECT_NEAR(actual.x, expected.x, 1e-4F);
		EXPECT_NEAR(actual.y, expected.y, 1e-4F);
		EXPECT_NEAR(actual.z, expected.z, 1e-4F);
	}

	TEST(Transform, asMatrixAppliesScaleBeforeRotation) {
		using namespace hg::math;

		// Scale (2, 1, 1) stretches x first; a 90Deg yaw then maps the stretched x axis onto z.
		const auto t = Transform { Location {}, Rotator { 0.F, 1.5707964F, 0.F }, Vector3f32 { 2.F, 1.F, 1.F } };

		const auto actual = t.asMatrix() * fvec4 { 1.F, 0.F, 0.F, 0.F };

		EXPECT_NEAR(actual.x, 0.F, 1e-4F);
		EXPECT_NEAR(actual.y, 0.F, 1e-4F);
		EXPECT_NEAR(std::abs(actual.z), 2.F, 1e-4F);
	}
}
