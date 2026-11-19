#include "../pch.h"

/**/

#include "../Harness/FunctionTarget.hpp"
#include "../Harness/GpuTest.hpp"

import std;

using namespace hg::test::gpu;
using namespace hg;

namespace hg::math {
	// Found by gtest through ADL, so a texel mismatch prints values instead of raw bytes.
	static void PrintTo(ref<const fvec4> value_, ptr<std::ostream> os_) {
		*os_ << '(' << value_.x << ", " << value_.y << ", " << value_.z << ", " << value_.w << ')';
	}
}

namespace {
	class Readback :
		public GpuTest {};
}

TEST_F(Readback, ClearReadsBack) {

	const auto value = math::fvec4 { 0.25F, -0.5F, 1e-3F, 3.0F };

	FunctionTarget target { device(), math::uivec2 { 32uL, 32uL } };
	target.clear(value);
	const auto raster = target.read();

	ASSERT_EQ(raster.texels.size(), 32uLL * 32uLL);
	for (u32 y = 0; y < raster.extent.y; ++y) {
		for (u32 x = 0; x < raster.extent.x; ++x) {
			ASSERT_EQ(raster.at(x, y), value) << x << ", " << y;
		}
	}
}

TEST_F(Readback, OddExtent) {

	FunctionTarget first { device(), math::uivec2 { 33uL, 17uL } };
	first.clear(math::fvec4 { sentinel });

	FunctionTarget second { device(), math::uivec2 { 33uL, 17uL } };
	second.clear(math::fvec4 { 1.0F, 2.0F, 3.0F, 4.0F });

	const auto raster = first.read();

	ASSERT_EQ(first.extent(), (math::uivec2 { 33uL, 17uL }));
	ASSERT_EQ(raster.extent, (math::uivec2 { 33uL, 17uL }));
	ASSERT_EQ(raster.texels.size(), 561uLL);
	for (const auto& texel : raster.texels) {
		ASSERT_EQ(texel, math::fvec4 { sentinel });
	}
}
