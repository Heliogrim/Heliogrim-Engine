#include "pch.h"

#include <Engine.Common/Math/__default.inl>

using namespace hg;

namespace Math {
	namespace Vector {
		TEST(Vector1f32, ZeroChecks) {
			using namespace ::hg::math;

			constexpr auto value = 236.5894656f;
			constexpr auto base_subject = Vector1f32 { value };

			EXPECT_FALSE(base_subject.allZero());
			EXPECT_FALSE(base_subject.anyZero());

			const auto subtract_subject = base_subject - value;

			EXPECT_TRUE(subtract_subject.allZero());
			EXPECT_TRUE(subtract_subject.anyZero());

			const auto zero_subject = Vector1f32 { 0.f };

			EXPECT_TRUE(zero_subject.allZero());
			EXPECT_TRUE(zero_subject.anyZero());
		}

		TEST(Vector1f32, Equality) {
			using namespace ::hg::math;

			const auto value = 236.5894656f;
			const auto base_subject = Vector1f32 { value };
			const auto base_false_candidate = Vector1f32 { 0.3f };
			const auto base_true_candidate = Vector1f32 { 0.f + value };

			EXPECT_NE(base_subject, base_false_candidate);
			EXPECT_EQ(base_subject, base_true_candidate);

			const auto subtract_subject = base_subject - value;
			const auto subtract_false_candidate = Vector1f32 { 0.3f };
			const auto subtract_true_candidate = Vector1f32 { 0.f };

			EXPECT_NE(subtract_subject, subtract_false_candidate);
			EXPECT_EQ(subtract_subject, subtract_true_candidate);

			const auto zero_subject = Vector1f32 { 0.f };
			const auto zero_false_subject = Vector1f32 { 0.f + std::numeric_limits<f32>::epsilon() };
			const auto zero_true_subject = Vector1f32 { 0.f };

			EXPECT_NE(zero_subject, zero_false_subject);
			EXPECT_EQ(zero_subject, zero_true_subject);
		}
	}
}
