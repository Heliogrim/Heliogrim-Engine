#include "pch.h"

#include <Engine.Common/Math/Extent.hpp>

using namespace hg;

namespace Common {
	TEST(ExtentAlias, mapToExpectedTypes) {
		using namespace hg::math;

		static_assert(std::is_same_v<Extent2f32, Extent2D<f32>>);
		static_assert(std::is_same_v<Extent2i32, Extent2D<s32>>);
		static_assert(std::is_same_v<Extent2u32, Extent2D<u32>>);
		static_assert(std::is_same_v<Extent3f32, Extent3D<f32>>);
		static_assert(std::is_same_v<Extent3i32, Extent3D<s32>>);
		static_assert(std::is_same_v<Extent3u32, Extent3D<u32>>);
	}
}
