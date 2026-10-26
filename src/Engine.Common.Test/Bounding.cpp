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

	/* Two spheres intersect, if the distance of their centers is at most the sum of their radii. */

	TEST(BoundingSpherical, TouchingSpheresIntersect) {
		const Bounding a { vec3 { 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };
		// Centers are 2 apart and both radii are 1, so the surfaces touch in exactly one point.
		const Bounding touching { vec3 { 2.F, 0.F, 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };

		EXPECT_TRUE(a.intersectsSpherical(touching));
		// Center distance 3 equals the radii sum 1 + 2.
		EXPECT_TRUE(a.intersectsSpherical(vec3 { 3.F, 0.F, 0.F }, 2.F));
	}

	TEST(BoundingSpherical, PartiallyOverlappingSpheresIntersect) {
		const Bounding a { vec3 { 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };
		// Distance 1.5 is below the radii sum 2, but above each single radius (1).
		const Bounding overlapping { vec3 { 1.5F, 0.F, 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };

		EXPECT_TRUE(a.intersectsSpherical(overlapping));
	}

	TEST(BoundingSpherical, SeparatedSpheresDoNotIntersect) {
		const Bounding a { vec3 { 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };
		// Distance 2.5 exceeds the radii sum 2.
		const Bounding apart { vec3 { 2.5F, 0.F, 0.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };

		EXPECT_FALSE(a.intersectsSpherical(apart));
		EXPECT_FALSE(a.intersectsSpherical(vec3 { 2.5F, 0.F, 0.F }, 1.F));
	}

	TEST(BoundingSpherical, DifferentRadiiAreAddedNotSquaredAndAdded) {
		const Bounding big { vec3 { 0.F }, vec3 { 3.F, 0.F, 0.F }, 3.F };

		// Radii 3 and 1 => sum 4. The squared radii only add up to 10, i.e. a reach of about 3.16.
		EXPECT_TRUE(big.intersectsSpherical(vec3 { 4.F, 0.F, 0.F }, 1.F));
		EXPECT_FALSE(big.intersectsSpherical(vec3 { 4.5F, 0.F, 0.F }, 1.F));
	}

	TEST(BoundingSpherical, SphereInsideAnotherSphereIntersects) {
		const Bounding big { vec3 { 0.F }, vec3 { 3.F, 0.F, 0.F }, 3.F };

		// Small sphere (radius 1) sits fully inside the big one (radius 3), no surface contact.
		EXPECT_TRUE(big.intersectsSpherical(vec3 { 1.F, 0.F, 0.F }, 1.F));
	}

	TEST(BoundingSpherical, IntersectionIsIndependentOfAxisAndOffset) {
		const Bounding a { vec3 { 1.F, 2.F, 3.F }, vec3 { 1.F, 0.F, 0.F }, 1.F };

		// Offset (0, 2, 0) from `a`'s center is exactly the radii sum 1 + 1, along a different axis.
		EXPECT_TRUE(a.intersectsSpherical(vec3 { 1.F, 4.F, 3.F }, 1.F));
		EXPECT_FALSE(a.intersectsSpherical(vec3 { 1.F, 4.5F, 3.F }, 1.F));
	}

	TEST(BoundingSpherical, ContainsRequiresSmallerSphereInside) {
		const Bounding bounds { vec3 { 0.F }, vec3 { 10.F, 0.F, 0.F }, 10.F };

		EXPECT_TRUE(bounds.containsSpherical(vec3 { 0.F }, 1.F));
		EXPECT_FALSE(bounds.containsSpherical(vec3 { 0.F }, 11.F));
		EXPECT_FALSE(bounds.containsSpherical(vec3 { 100.F, 0.F, 0.F }, 1.F));
	}

	/* Containment: the small sphere (radius r) must lie fully within the big one (radius L), i.e. distance + r <= L. */

	TEST(BoundingSpherical, ContainsSphereTouchingTheInside) {
		const Bounding bounds { vec3 { 0.F }, vec3 { 10.F, 0.F, 0.F }, 10.F };

		// Center distance 9 plus radius 1 reaches exactly 10, so the small sphere touches the big one from inside.
		EXPECT_TRUE(bounds.containsSpherical(vec3 { 9.F, 0.F, 0.F }, 1.F));
	}

	TEST(BoundingSpherical, DoesNotContainSphereReachingOutside) {
		const Bounding bounds { vec3 { 0.F }, vec3 { 10.F, 0.F, 0.F }, 10.F };

		// Center distance 9.5 plus radius 1 reaches 10.5 and sticks out by 0.5, even though 9.5^2 + 1^2 < 10^2.
		EXPECT_FALSE(bounds.containsSpherical(vec3 { 9.5F, 0.F, 0.F }, 1.F));
	}

	TEST(BoundingSpherical, ContainsConcentricSphereOfSameRadius) {
		const Bounding bounds { vec3 { 0.F }, vec3 { 10.F, 0.F, 0.F }, 10.F };

		EXPECT_TRUE(bounds.containsSpherical(vec3 { 0.F }, 10.F));
		EXPECT_FALSE(bounds.containsSpherical(vec3 { 0.F }, 10.5F));
	}

	TEST(BoundingSpherical, ContainmentIsMeasuredFromOffsetCenter) {
		const Bounding bounds { vec3 { 5.F, 5.F, 5.F }, vec3 { 4.F, 0.F, 0.F }, 4.F };

		// Along y: distance 3 from the center (5, 5, 5), plus radius 1 reaches exactly 4.
		EXPECT_TRUE(bounds.containsSpherical(vec3 { 5.F, 8.F, 5.F }, 1.F));
		EXPECT_FALSE(bounds.containsSpherical(vec3 { 5.F, 8.5F, 5.F }, 1.F));
	}

	TEST(BoundingTest, TwoArgumentConstructorDerivesLength) {
		// Extent (3, 4, 0) spans the classic 3-4-5 triangle, so the enclosing sphere radius is exactly 5.
		const Bounding bounds { vec3 { 0.F }, vec3 { 3.F, 4.F, 0.F } };

		EXPECT_FLOAT_EQ(bounds.extentLength(), 5.F);
	}

	TEST(BoundingTest, TwoArgumentConstructorWithZeroExtentHasZeroLength) {
		const Bounding bounds { vec3 { 7.F, 7.F, 7.F }, vec3 { 0.F } };

		EXPECT_FLOAT_EQ(bounds.extentLength(), 0.F);
	}

	TEST(BoundingTest, ThreeArgumentConstructorKeepsGivenLength) {
		// The explicit length is trusted and not re-derived: (3, 4, 0) would give 5, but 10 is requested.
		const Bounding bounds { vec3 { 0.F }, vec3 { 3.F, 4.F, 0.F }, 10.F };

		EXPECT_FLOAT_EQ(bounds.extentLength(), 10.F);
	}

	TEST(BoundingSpherical, TwoArgumentConstructorIsUsableForSphereChecks) {
		// Radius is derived as 5 (3-4-5), so a point at distance 4 is inside and a point at distance 6 is outside.
		const Bounding bounds { vec3 { 0.F }, vec3 { 3.F, 4.F, 0.F } };

		EXPECT_TRUE(bounds.intersectsSpherical(vec3 { 4.F, 0.F, 0.F }));
		EXPECT_FALSE(bounds.intersectsSpherical(vec3 { 6.F, 0.F, 0.F }));
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

	/* Point vs. box: the box spans [center - extent, center + extent] on each axis, borders included. */

	TEST(BoundingAabb, PointInsideBoxIntersects) {
		const Bounding box { vec3 { 0.F }, vec3 { 1.F } };

		EXPECT_TRUE(box.intersectsAaBb(vec3 { 0.F }));
		EXPECT_TRUE(box.intersectsAaBb(vec3 { 0.5F, -0.5F, 0.5F }));
	}

	TEST(BoundingAabb, PointOnBoxBorderIntersects) {
		const Bounding box { vec3 { 0.F }, vec3 { 1.F } };

		EXPECT_TRUE(box.intersectsAaBb(vec3 { 1.F, 0.F, 0.F }));
		EXPECT_TRUE(box.intersectsAaBb(vec3 { -1.F, -1.F, -1.F }));
	}

	TEST(BoundingAabb, PointOutsideOnSingleAxisDoesNotIntersect) {
		const Bounding box { vec3 { 0.F }, vec3 { 1.F } };

		// Each point is inside the box on two axes and just beyond the border (1.5 > 1) on the remaining one.
		EXPECT_FALSE(box.intersectsAaBb(vec3 { 1.5F, 0.F, 0.F }));
		EXPECT_FALSE(box.intersectsAaBb(vec3 { 0.F, -1.5F, 0.F }));
		EXPECT_FALSE(box.intersectsAaBb(vec3 { 0.F, 0.F, 1.5F }));
	}

	TEST(BoundingAabb, PointVsOffCenterBoxWithDistinctExtents) {
		// Box spans x [9, 11], y [-2, 2], z [-3, 3].
		const Bounding box { vec3 { 10.F, 0.F, 0.F }, vec3 { 1.F, 2.F, 3.F } };

		EXPECT_TRUE(box.intersectsAaBb(vec3 { 10.5F, -1.5F, 2.5F }));
		EXPECT_FALSE(box.intersectsAaBb(vec3 { 11.5F, 0.F, 0.F }));
		EXPECT_FALSE(box.intersectsAaBb(vec3 { 10.F, 2.5F, 0.F }));
		EXPECT_FALSE(box.intersectsAaBb(vec3 { 10.F, 0.F, 3.5F }));
		// The world origin is far away from the box, even though it is inside the box's extent magnitude.
		EXPECT_FALSE(box.intersectsAaBb(vec3 { 0.F }));
	}

	TEST(BoundingAabb, DegenerateBoxOnlyContainsItsCenterPoint) {
		const Bounding degenerate { vec3 { 0.F }, vec3 { 0.F } };

		EXPECT_TRUE(degenerate.intersectsAaBb(vec3 { 0.F }));
		EXPECT_FALSE(degenerate.intersectsAaBb(vec3 { 0.1F, 0.F, 0.F }));
	}
}
