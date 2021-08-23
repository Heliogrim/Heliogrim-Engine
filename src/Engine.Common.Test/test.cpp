#include "pch.h"

#include <Engine.Common/Math/Transformation.hpp>
#include <Engine.Common/Math/Vector.hpp>

using namespace ember;

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

TEST(MathVectorTest, vectorInit2d) {
		using namespace ember::math;
		const int x = 10;
		const int y = 35;

		const vec2_t<int> test = vec2_t<int>(x, y);

		EXPECT_EQ(x, test.x);
		EXPECT_EQ(y, test.y);
	}

	TEST(MathVectorTest, vectorDownCast) {
		using namespace ember::math;
		const vec3_t<int> src = vec3_t<int>(30, 40, 50);
		const vec2_t<int> dst = vec2_t<int>(src);

		EXPECT_EQ(src.x, dst.x);
		EXPECT_EQ(src.y, dst.y);
	}

	TEST(MathVectorTest, vectorUpCast) {
		using namespace ember::math;
		const vec3_t<int> src = vec3_t<int>(70, 90, 110);
		const vec4_t<int> dst = vec4_t<int>(src, 5);

		EXPECT_EQ(src.x, dst.x);
		EXPECT_EQ(src.y, dst.y);
		EXPECT_EQ(src.z, dst.z);
	}

	TEST(MathVectorTest, vectorNormalize) {
		using namespace ember::math;
		vec3_t<float> v = vec3_t<float>(5.F);
		v.normalize();

		EXPECT_FLOAT_EQ(v.x, 1.F / sqrt(3.F));
		EXPECT_FLOAT_EQ(v.y, 1.F / sqrt(3.F));
		EXPECT_FLOAT_EQ(v.z, 1.F / sqrt(3.F));
	}

	TEST(Transform, initZero) {
		using namespace ember::math;
		Transformation t = ZeroTransformation();

		EXPECT_EQ(t.position(), vec3());
		EXPECT_EQ(t.rotation(), vec3());
		EXPECT_EQ(t.scale(), vec3(1.F));
	}

	TEST(Quaternion, fromToEuler) {
		using namespace ember::math;
		const float r45f = glm::radians(45.F);
		const float r0f = glm::radians(0.F);
		
		quaternion q = quaternion::euler({
			r45f, r0f, r0f
		});

		const vec3 ve = q.euler();
		EXPECT_FLOAT_EQ(ve.x, r45f);
		EXPECT_FLOAT_EQ(ve.y, r0f);
		EXPECT_FLOAT_EQ(ve.z, r0f);
	}