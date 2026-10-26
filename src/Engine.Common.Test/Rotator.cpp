#include "pch.h"

#include <array>
#include <numbers>
#include <Engine.Common/Hash/Fnv-1a.hpp>
#include <Engine.Common/Math/Bezier.hpp>
#include <Engine.Common/Math/Hash.hpp>
#include <Engine.Common/Math/Rotator.hpp>

using namespace hg;
using namespace hg::math;

namespace Common {
	namespace {
		constexpr auto pi = std::numbers::pi_v<float>;
	}

	/* Rotator */

	TEST(RotatorTest, ComponentAccessors) {
		const Rotator rot { 1.F, 2.F, 3.F };

		EXPECT_EQ(rot.pitch(), 1.F);
		EXPECT_EQ(rot.yaw(), 2.F);
		EXPECT_EQ(rot.roll(), 3.F);
		EXPECT_EQ(rot.euler(), (fvec3 { 1.F, 2.F, 3.F }));
	}

	TEST(RotatorTest, SettersAreChainable) {
		Rotator rot {};

		rot.setPitch(1.F).setYaw(2.F).setRoll(3.F);

		EXPECT_EQ(rot, (Rotator { 1.F, 2.F, 3.F }));
	}

	TEST(RotatorTest, EqualityComparesAllAxes) {
		EXPECT_TRUE((Rotator { 1.F, 2.F, 3.F } == Rotator { 1.F, 2.F, 3.F }));
		EXPECT_TRUE((Rotator { 1.F, 2.F, 3.F } != Rotator { 1.F, 2.F, 4.F }));
	}

	TEST(RotatorTest, ArithmeticOperators) {
		const Rotator a { 1.F, 2.F, 3.F };
		const Rotator b { 0.5F, 0.5F, 0.5F };

		EXPECT_EQ(a + b, (Rotator { 1.5F, 2.5F, 3.5F }));
		EXPECT_EQ(a - b, (Rotator { 0.5F, 1.5F, 2.5F }));
		EXPECT_EQ(a * 2.F, (Rotator { 2.F, 4.F, 6.F }));
	}

	TEST(RotatorTest, CompoundOperators) {
		Rotator rot { 1.F, 1.F, 1.F };

		rot += Rotator { 1.F, 2.F, 3.F };
		EXPECT_EQ(rot, (Rotator { 2.F, 3.F, 4.F }));

		rot -= Rotator { 1.F, 1.F, 1.F };
		EXPECT_EQ(rot, (Rotator { 1.F, 2.F, 3.F }));

		rot *= 3.F;
		EXPECT_EQ(rot, (Rotator { 3.F, 6.F, 9.F }));
	}

	TEST(RotatorTest, InverseNegatesAxes) {
		const Rotator rot { 1.F, -2.F, 3.F };

		EXPECT_EQ(rot.inversedRotator(), (Rotator { -1.F, 2.F, -3.F }));

		Rotator mutated = rot;
		mutated.inverseRotator();
		EXPECT_EQ(mutated, rot.inversedRotator());
	}

	TEST(RotatorTest, NormalizeAxisWrapsIntoMinusPiToPi) {
		const Rotator rot {};

		EXPECT_NEAR(rot.normalizeAxis(0.F), 0.F, 1e-5F);
		EXPECT_NEAR(rot.normalizeAxis(pi + 0.5F), -pi + 0.5F, 1e-4F);
		EXPECT_NEAR(rot.normalizeAxis(-pi - 0.5F), pi - 0.5F, 1e-4F);
		EXPECT_NEAR(rot.normalizeAxis(2.F * pi + 0.25F), 0.25F, 1e-4F);
	}

	TEST(RotatorTest, DenormalizeAxisWrapsIntoZeroToTwoPi) {
		const Rotator rot {};

		EXPECT_NEAR(rot.denormalizeAxis(-0.5F), 2.F * pi - 0.5F, 1e-4F);
		EXPECT_NEAR(rot.denormalizeAxis(2.F * pi + 0.5F), 0.5F, 1e-4F);
		EXPECT_NEAR(rot.denormalizeAxis(1.F), 1.F, 1e-5F);
	}

	TEST(RotatorTest, NormalizedRotatorAppliesToAllAxes) {
		const Rotator rot { 2.F * pi + 0.25F, -2.F * pi - 0.25F, 0.5F };

		const auto normalized = rot.normalizedRotator();

		EXPECT_NEAR(normalized.pitch(), 0.25F, 1e-4F);
		EXPECT_NEAR(normalized.yaw(), -0.25F, 1e-4F);
		EXPECT_NEAR(normalized.roll(), 0.5F, 1e-5F);
	}

	TEST(RotatorTest, InPlaceNormalizeMatchesCopy) {
		Rotator rot { 4.F, -4.F, 7.F };
		const auto copy = rot.normalizedRotator();

		rot.normalizeRotator();

		EXPECT_EQ(rot, copy);
	}

	TEST(RotatorTest, InPlaceDenormalizeMatchesCopy) {
		Rotator rot { 4.F, -4.F, 7.F };
		const auto copy = rot.denormalizedRotator();

		rot.denormalizeRotator();

		EXPECT_EQ(rot, copy);
	}

	TEST(RotatorTest, ZeroRotationKeepsVector) {
		const Rotator rot {};
		const fvec3 vec { 1.F, 2.F, 3.F };

		const auto rotated = rot.rotated(vec);

		EXPECT_NEAR(rotated.x, 1.F, 1e-5F);
		EXPECT_NEAR(rotated.y, 2.F, 1e-5F);
		EXPECT_NEAR(rotated.z, 3.F, 1e-5F);
	}

	TEST(RotatorTest, RotationPreservesLength) {
		const Rotator rot { 0.3F, 0.7F, -0.2F };
		const fvec3 vec { 1.F, 2.F, 3.F };

		const auto rotated = rot.rotated(vec);

		EXPECT_NEAR(rotated.length(), vec.length(), 1e-4F);
	}


	TEST(RotatorTest, InPlaceRotateMatchesCopy) {
		const Rotator rot { 0.3F, 0.7F, -0.2F };
		fvec3 vec { 1.F, 2.F, 3.F };
		const auto expected = rot.rotated(vec);

		rot.rotate(vec);

		EXPECT_NEAR(vec.x, expected.x, 1e-5F);
		EXPECT_NEAR(vec.y, expected.y, 1e-5F);
		EXPECT_NEAR(vec.z, expected.z, 1e-5F);
	}



	TEST(RotatorTest, DiffOfEqualRotationsIsIdentity) {
		const Rotator rot { 0.3F, 0.4F, 0.2F };

		const auto diff = Rotator::diff(rot, rot);

		EXPECT_NEAR(diff.pitch(), 0.F, 1e-3F);
		EXPECT_NEAR(diff.yaw(), 0.F, 1e-3F);
		EXPECT_NEAR(diff.roll(), 0.F, 1e-3F);
	}


	TEST(RotatorTest, FromEulerRawKeepsAxes) {
		const auto rot = Rotator::fromEulerRaw(fvec3 { 1.F, 2.F, 3.F });

		EXPECT_EQ(rot.pitch(), 1.F);
		EXPECT_EQ(rot.yaw(), 2.F);
		EXPECT_EQ(rot.roll(), 3.F);
	}

	TEST(RotatorTest, InverseRotationUndoesSingleAxisRotation) {
		const fvec3 vec { 1.F, 2.F, 3.F };

		for (const auto& rot : { Rotator { 0.7F, 0.F, 0.F }, Rotator { 0.F, 0.7F, 0.F }, Rotator { 0.F, 0.F, 0.7F } }) {
			const auto roundTrip = rot.inversed(rot.rotated(vec));

			EXPECT_NEAR(roundTrip.x, vec.x, 1e-4F);
			EXPECT_NEAR(roundTrip.y, vec.y, 1e-4F);
			EXPECT_NEAR(roundTrip.z, vec.z, 1e-4F);
		}
	}

	TEST(RotatorTest, InverseRotationUndoesCombinedRotation) {
		const Rotator rot { 0.3F, 0.7F, -0.2F };
		const fvec3 vec { 1.F, 2.F, 3.F };

		const auto roundTrip = rot.inversed(rot.rotated(vec));

		EXPECT_NEAR(roundTrip.x, vec.x, 1e-3F);
		EXPECT_NEAR(roundTrip.y, vec.y, 1e-3F);
		EXPECT_NEAR(roundTrip.z, vec.z, 1e-3F);
	}

	TEST(RotatorTest, QuaternionRoundTripSingleAxis) {
		for (const auto& rot : { Rotator { 0.3F, 0.F, 0.F }, Rotator { 0.F, 0.3F, 0.F }, Rotator { 0.F, 0.F, 0.3F } }) {
			const auto back = Rotator { rot.into() };

			EXPECT_NEAR(back.pitch(), rot.pitch(), 1e-4F);
			EXPECT_NEAR(back.yaw(), rot.yaw(), 1e-4F);
			EXPECT_NEAR(back.roll(), rot.roll(), 1e-4F);
		}
	}

	/* Rotator -> quaternion -> Rotator must give back the same angles, as long as yaw stays within (-90Deg, 90Deg). */

	TEST(RotatorTest, QuaternionRoundTripCombinedAxes) {
		const Rotator rot { 0.3F, 0.4F, 0.2F };

		const auto back = Rotator { rot.into() };

		EXPECT_NEAR(back.pitch(), 0.3F, 1e-3F);
		EXPECT_NEAR(back.yaw(), 0.4F, 1e-3F);
		EXPECT_NEAR(back.roll(), 0.2F, 1e-3F);
	}

	/* Rotator -> rotation matrix -> Rotator is the quaternion-free way back. */

	TEST(RotatorTest, RotationMatrixRoundTripCombinedAxes) {
		const auto rot = Rotator { 0.3F, 0.4F, 0.2F };

		const auto back = Rotator::fromRotationMatrix(rot.asRotationMatrix());

		EXPECT_NEAR(back.pitch(), 0.3F, 1e-4F);
		EXPECT_NEAR(back.yaw(), 0.4F, 1e-4F);
		EXPECT_NEAR(back.roll(), 0.2F, 1e-4F);
	}

	TEST(RotatorTest, RotationMatrixRoundTripNegativeAndLargeAngles) {
		// Pitch and roll may use the full (-180Deg, 180Deg) range; 2.5 rad is about 143Deg.
		const auto rot = Rotator { -2.5F, -0.6F, 2.5F };

		const auto back = Rotator::fromRotationMatrix(rot.asRotationMatrix());

		EXPECT_NEAR(back.pitch(), -2.5F, 1e-4F);
		EXPECT_NEAR(back.yaw(), -0.6F, 1e-4F);
		EXPECT_NEAR(back.roll(), 2.5F, 1e-4F);
	}

	TEST(RotatorTest, RotationMatrixOfIdentityIsIdentity) {
		const auto matrix = Rotator {}.asRotationMatrix();

		// Row-major identity: ones on the diagonal (indices 0, 4, 8), zeros elsewhere.
		const auto identity = std::array<float, 9> { 1.F, 0.F, 0.F, 0.F, 1.F, 0.F, 0.F, 0.F, 1.F };
		for (auto i = 0uLL; i < 9uLL; ++i) {
			EXPECT_NEAR(matrix[i], identity[i], 1e-6F);
		}
	}

	TEST(RotatorTest, RotationMatrixOfQuarterYawMapsForwardOntoRight) {
		// Y is up, so a +90Deg yaw turns Z (forward) towards +X (right): Ry maps (0, 0, 1) onto (1, 0, 0).
		const auto matrix = Rotator { 0.F, 1.5707964F, 0.F }.asRotationMatrix();

		// Third column of the row-major matrix is (m[2], m[5], m[8]).
		EXPECT_NEAR(matrix[2], 1.F, 1e-5F);
		EXPECT_NEAR(matrix[5], 0.F, 1e-5F);
		EXPECT_NEAR(matrix[8], 0.F, 1e-5F);
	}

	TEST(RotatorTest, RotationMatrixRoundTripAtGimbalLockKeepsOrientation) {
		// Yaw of exactly +90Deg: only the sum of pitch and roll is defined, so compare the orientation instead.
		const auto rot = Rotator { 0.3F, 1.5707964F, 0.2F };
		const auto vec = fvec3 { 1.F, 2.F, 3.F };

		const auto back = Rotator::fromRotationMatrix(rot.asRotationMatrix());

		const auto expected = rot.rotated(vec);
		const auto actual = back.rotated(vec);
		EXPECT_NEAR(actual.x, expected.x, 1e-3F);
		EXPECT_NEAR(actual.y, expected.y, 1e-3F);
		EXPECT_NEAR(actual.z, expected.z, 1e-3F);
	}

	TEST(RotatorTest, QuaternionRoundTripNegativeAndLargeAngles) {
		// Pitch and roll may use the full (-180Deg, 180Deg) range; 2.5 rad is about 143Deg.
		const auto rot = Rotator { -2.5F, -0.6F, 2.5F };

		const auto back = Rotator { rot.into() };

		EXPECT_NEAR(back.pitch(), -2.5F, 1e-3F);
		EXPECT_NEAR(back.yaw(), -0.6F, 1e-3F);
		EXPECT_NEAR(back.roll(), 2.5F, 1e-3F);
	}

	TEST(RotatorTest, QuaternionRoundTripAtGimbalLockKeepsOrientation) {
		// Yaw of exactly 90Deg: pitch and roll turn around the same axis, so only their combined effect is defined.
		const auto rot = Rotator { 0.3F, 1.5707964F, 0.F };
		const auto vec = fvec3 { 1.F, 2.F, 3.F };

		const auto back = Rotator { rot.into() };

		// Compare the orientation through a rotated vector instead of the (ambiguous) angles.
		const auto expected = rot.rotated(vec);
		const auto actual = back.rotated(vec);
		EXPECT_NEAR(actual.x, expected.x, 1e-3F);
		EXPECT_NEAR(actual.y, expected.y, 1e-3F);
		EXPECT_NEAR(actual.z, expected.z, 1e-3F);
	}

	TEST(RotatorTest, QuaternionConversionMatchesMatrixRotationOfVector) {
		// The quaternion path and the matrix path of a combined rotation have to describe the same orientation.
		const auto rot = Rotator { 0.3F, 0.7F, -0.2F };
		const auto vec = fvec3 { 1.F, 2.F, 3.F };

		const auto viaMatrix = rot.rotated(vec);
		const auto viaRoundTrip = Rotator { rot.into() }.rotated(vec);

		EXPECT_NEAR(viaRoundTrip.x, viaMatrix.x, 1e-3F);
		EXPECT_NEAR(viaRoundTrip.y, viaMatrix.y, 1e-3F);
		EXPECT_NEAR(viaRoundTrip.z, viaMatrix.z, 1e-3F);
	}

	TEST(RotatorTest, CombineWithIdentityKeepsSingleAxisRotation) {
		const Rotator rot { 0.3F, 0.F, 0.F };

		const auto combined = Rotator::combine(rot, Rotator {});

		EXPECT_NEAR(combined.pitch(), rot.pitch(), 1e-4F);
		EXPECT_NEAR(combined.yaw(), 0.F, 1e-4F);
		EXPECT_NEAR(combined.roll(), 0.F, 1e-4F);
	}

	TEST(RotatorTest, OuterToInnerUndoesInnerToOuterSingleAxis) {
		const Rotator subject { 0.1F, 0.F, 0.F };
		const Rotator outer { 0.3F, 0.F, 0.F };

		const auto back = Rotator::outerToInner(Rotator::innerToOuter(subject, outer), outer);

		EXPECT_NEAR(back.pitch(), subject.pitch(), 1e-4F);
		EXPECT_NEAR(back.yaw(), 0.F, 1e-4F);
		EXPECT_NEAR(back.roll(), 0.F, 1e-4F);
	}

	/* Bezier */

	TEST(BezierTest, EndpointsAreInterpolated) {
		const bezier<float> curve { 1.F, 5.F, -3.F, 9.F };

		EXPECT_FLOAT_EQ(curve(0.F), 1.F);
		EXPECT_FLOAT_EQ(curve(1.F), 9.F);
	}

	TEST(BezierTest, LinearControlPointsInterpolateLinearly) {
		const bezier<float> curve { 0.F, 1.F, 2.F, 3.F };

		EXPECT_NEAR(curve(0.25F), 0.75F, 1e-5F);
		EXPECT_NEAR(curve(0.5F), 1.5F, 1e-5F);
	}

	TEST(BezierTest, MidpointWeightsControlPoints) {
		const bezier<float> curve { 0.F, 0.F, 8.F, 8.F };

		EXPECT_NEAR(curve(0.5F), 4.F, 1e-5F);
	}

	/* Hash */

	TEST(HashCombineTest, IsDeterministic) {
		size_t a = 0uLL, b = 0uLL;

		hash::hash_combine(a, 42uLL);
		hash::hash_combine(b, 42uLL);

		EXPECT_EQ(a, b);
		EXPECT_NE(a, 0uLL);
	}

	TEST(HashCombineTest, OrderMatters) {
		size_t ab = 0uLL, ba = 0uLL;

		hash::hash_combine(ab, 1uLL);
		hash::hash_combine(ab, 2uLL);
		hash::hash_combine(ba, 2uLL);
		hash::hash_combine(ba, 1uLL);

		EXPECT_NE(ab, ba);
	}

	TEST(HashCombineTest, DifferentValuesDiffer) {
		size_t a = 0uLL, b = 0uLL;

		hash::hash_combine(a, 1uLL);
		hash::hash_combine(b, 2uLL);

		EXPECT_NE(a, b);
	}

	TEST(HashCombineTest, VariadicMatchesSequentialImpl) {
		u64 variadic = 0uLL, sequential = 0uLL;

		hash::hashCombine(variadic, 1uLL, 2uLL, 3uLL);
		hash::hashCombineImpl(sequential, 1uLL);
		hash::hashCombineImpl(sequential, 2uLL);
		hash::hashCombineImpl(sequential, 3uLL);

		EXPECT_EQ(variadic, sequential);
	}
}
