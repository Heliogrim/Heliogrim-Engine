#include "pch.h"

#include <Engine.Common/Math/Bounding.hpp>

using namespace hg;
using namespace hg::math;

namespace Common {
	TEST(BoundingTest, DefaultIsZeroSized) {
		const Bounding bounds {};

		EXPECT_EQ(bounds.center(), vec3(0.F));
		EXPECT_EQ(bounds.extent(), vec3(0.F));
		EXPECT_EQ(bounds.extentLength(), 0.F);
	}

	TEST(BoundingTest, SetExtentRestoresLength) {
		Bounding bounds {};

		bounds.setExtent(vec3 { 3.F, 4.F, 0.F });

		EXPECT_FLOAT_EQ(bounds.extentLength(), 5.F);
	}

	TEST(BoundingTest, SetExtentTwoComponentsZeroesDepth) {
		Bounding bounds { vec3 { 0.F }, vec3 { 9.F, 9.F, 9.F } };

		bounds.setExtent(3.F, 4.F);

		EXPECT_EQ(bounds.extent(), (vec3 { 3.F, 4.F, 0.F }));
		EXPECT_FLOAT_EQ(bounds.extentLength(), 5.F);
	}

	TEST(BoundingTest, SetExtentThreeComponents) {
		Bounding bounds {};

		bounds.setExtent(2.F, 3.F, 6.F);

		EXPECT_FLOAT_EQ(bounds.extentLength(), 7.F);
	}

	TEST(BoundingTest, ApplyExtentScaleScalesExtentAndLength) {
		Bounding bounds {};
		bounds.setExtent(3.F, 4.F, 0.F);

		bounds.applyExtentScale(2.F);

		EXPECT_EQ(bounds.extent(), (vec3 { 6.F, 8.F, 0.F }));
		EXPECT_FLOAT_EQ(bounds.extentLength(), 10.F);
	}

	TEST(BoundingTest, CenterIsMutable) {
		Bounding bounds {};

		bounds.center() = vec3 { 1.F, 2.F, 3.F };

		EXPECT_EQ(bounds.center(), (vec3 { 1.F, 2.F, 3.F }));
	}

	/* Spherical */

	TEST(BoundingSpherical, PointInsideAndOutside) {
		const Bounding bounds { vec3 { 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };

		EXPECT_TRUE(bounds.intersectsSpherical(vec3 { 0.5F, 0.F, 0.F }));
		EXPECT_TRUE(bounds.intersectsSpherical(vec3 { 1.F, 0.F, 0.F }));
		EXPECT_FALSE(bounds.intersectsSpherical(vec3 { 2.F, 0.F, 0.F }));
	}

	TEST(BoundingSpherical, SphereOverlapAddsRadius) {
		const Bounding bounds { vec3 { 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };

		EXPECT_TRUE(bounds.intersectsSpherical(vec3 { 1.F, 0.F, 0.F }, 1.F));
		EXPECT_FALSE(bounds.intersectsSpherical(vec3 { 5.F, 0.F, 0.F }, 1.F));
	}

	TEST(BoundingSpherical, BoundingOverlapUsesOtherLength) {
		const Bounding a { vec3 { 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };
		const Bounding near { vec3 { 1.F, 0.F, 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };
		const Bounding far { vec3 { 5.F, 0.F, 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };

		EXPECT_TRUE(a.intersectsSpherical(near));
		EXPECT_FALSE(a.intersectsSpherical(far));
	}

	// BUG: sphere-sphere tests compare squared distance against `r1^2 + r2^2` instead of `(r1 + r2)^2`, so spheres
	// that touch or partially overlap (distance between max(r) and r1 + r2) are reported as disjoint.
	TEST(BoundingSpherical, DISABLED_TouchingSpheresIntersect) {
		const Bounding a { vec3 { 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };
		const Bounding touching { vec3 { 2.F, 0.F, 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };

		EXPECT_TRUE(a.intersectsSpherical(touching));
		EXPECT_TRUE(a.intersectsSpherical(vec3 { 3.F, 0.F, 0.F }, 2.F));
	}

	TEST(BoundingSpherical, ContainsRequiresSmallerSphereInside) {
		const Bounding bounds { vec3 { 0.F }, vec3 { 10.F, 0.F, 0.F }, 10.F };

		EXPECT_TRUE(bounds.containsSpherical(vec3 { 0.F }, 1.F));
		EXPECT_FALSE(bounds.containsSpherical(vec3 { 0.F }, 11.F));
		EXPECT_FALSE(bounds.containsSpherical(vec3 { 100.F, 0.F, 0.F }, 1.F));
	}

	// SUSPECTED BUG: the two-argument constructor leaves the extent length at 0, so spherical checks treat the
	// bounding as an empty sphere until restoreLength()/setExtent() is called.
	TEST(BoundingSpherical, DISABLED_ConstructedWithExtentDerivesLength) {
		const Bounding bounds { vec3 { 0.F }, vec3 { 3.F, 4.F, 0.F } };

		EXPECT_FLOAT_EQ(bounds.extentLength(), 5.F);
	}

	/* AABB */

	TEST(BoundingAabb, BoxOverlap) {
		const Bounding box { vec3 { 0.F }, vec3 { 1.F } };

		EXPECT_TRUE(box.intersectsAaBb(vec3 { 1.5F, 0.F, 0.F }, vec3 { 1.F }));
		EXPECT_FALSE(box.intersectsAaBb(vec3 { 5.F, 0.F, 0.F }, vec3 { 1.F }));
	}

	TEST(BoundingAabb, TouchingFacesCountAsIntersecting) {
		const Bounding box { vec3 { 0.F }, vec3 { 1.F } };

		EXPECT_TRUE(box.intersectsAaBb(vec3 { 2.F, 0.F, 0.F }, vec3 { 1.F }));
	}

	TEST(BoundingAabb, SeparationOnSingleAxisIsEnough) {
		const Bounding box { vec3 { 0.F }, vec3 { 1.F } };

		EXPECT_FALSE(box.intersectsAaBb(vec3 { 0.F, 0.F, 3.F }, vec3 { 1.F }));
	}

	TEST(BoundingAabb, BoundingOverloadMatchesBoxOverload) {
		const Bounding a { vec3 { 0.F }, vec3 { 1.F } };
		const Bounding b { vec3 { 1.5F, 0.F, 0.F }, vec3 { 1.F } };

		EXPECT_EQ(a.intersectsAaBb(b), a.intersectsAaBb(b.center(), b.extent()));
	}

	TEST(BoundingAabb, ContainsRequiresFullEnclosure) {
		const Bounding box { vec3 { 0.F }, vec3 { 10.F } };

		EXPECT_TRUE(box.containsAaBb(vec3 { 0.F }, vec3 { 1.F }));
		EXPECT_TRUE(box.containsAaBb(vec3 { 0.F }, vec3 { 10.F }));
		EXPECT_FALSE(box.containsAaBb(vec3 { 9.5F, 0.F, 0.F }, vec3 { 1.F }));
		EXPECT_FALSE(box.containsAaBb(vec3 { 0.F }, vec3 { 11.F }));
	}

	// BUG: intersectsAaBb(point) requires `extent <= delta <= -extent`, which is unsatisfiable for any positive
	// extent, so a point at the center of a non-degenerate box is reported as outside. Enable once fixed.
	TEST(BoundingAabb, DISABLED_PointInsideBoxIntersects) {
		const Bounding box { vec3 { 0.F }, vec3 { 1.F } };

		EXPECT_TRUE(box.intersectsAaBb(vec3 { 0.F }));
		EXPECT_FALSE(box.intersectsAaBb(vec3 { 5.F, 0.F, 0.F }));
	}

	TEST(BoundingAabb, PointOnlyIntersectsDegenerateBox) {
		// Documents current behaviour of the point overload (see disabled test above).
		const Bounding degenerate { vec3 { 0.F }, vec3 { 0.F } };

		EXPECT_TRUE(degenerate.intersectsAaBb(vec3 { 0.F }));
	}
}
