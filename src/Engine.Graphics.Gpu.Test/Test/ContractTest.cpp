#include "../pch.h"

/**/

#include <limits>

#include "../Harness/Contract.hpp"

using namespace hg::test::gpu;
using namespace hg;

namespace {
	Raster make(const u32 w_, const u32 h_, const float v_ = 0.F) {
		return Raster { math::uivec2 { w_, h_ }, Vector<math::fvec4>(static_cast<size_t>(w_) * h_, math::fvec4 { v_ }) };
	}

	CheckResult run(ref<const Kernel> k_, const Raster& a_, const Raster& e_) {
		return k_(a_, e_);
	}
}

TEST(Contract, EqualPasses) {
	const auto a = make(4, 4, 0.5F);
	const auto r = check(Contract { { absEpsilon(1e-3F), relEpsilon(1e-3F) } }, a, a);
	ASSERT_EQ(r.size(), 2u);
	EXPECT_TRUE(r[0].passed);
	EXPECT_TRUE(r[1].passed);
	EXPECT_EQ(r[0].failing, 0u);
}

TEST(Contract, AbsEpsilonCountsAndLocates) {
	const auto e = make(4, 4);
	auto a = make(4, 4);
	a.texels[2 * 4 + 3].x = 0.1F;
	const auto r = run(absEpsilon(1e-3F), a, e);
	EXPECT_FALSE(r.passed);
	EXPECT_EQ(r.failing, 1u);
	ASSERT_EQ(r.failingTexels.size(), 1u);
	EXPECT_EQ(r.failingTexels[0], (math::uivec2 { 3, 2 }));
	EXPECT_NEAR(r.measured, 0.1F, 1e-6F);
}

TEST(Contract, RelEpsilonScales) {
	const auto e = make(2, 2, 1000.F);
	const auto a = make(2, 2, 1000.5F);
	EXPECT_TRUE(run(relEpsilon(1e-3F), a, e).passed);
	EXPECT_FALSE(run(relEpsilon(1e-4F), a, e).passed);
}

TEST(Contract, NanFails) {
	const auto e = make(2, 2);
	for (const auto bad : { std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity() }) {
		auto a = make(2, 2);
		a.texels[1].y = bad;
		EXPECT_FALSE(run(absEpsilon(1.F), a, e).passed);
		EXPECT_FALSE(run(relEpsilon(1.F), a, e).passed);
		EXPECT_FALSE(run(slopeEpsilon(Axis::eX, 1, 1.F), a, e).passed);
	}
}

TEST(Contract, MonotonicFindsViolation) {
	auto a = make(4, 2);
	for (u32 y = 0; y < 2; ++y) {
		for (u32 x = 0; x < 4; ++x) {
			a.texels[y * 4 + x].x = static_cast<float>(x);
		}
	}
	EXPECT_TRUE(run(monotonic(Axis::eX, 0, Order::eIncreasing), a, a).passed);
	a.texels[2].x = 0.5F;// row 0 dips: 0, 1, 0.5, 3
	const auto r = run(monotonic(Axis::eX, 0, Order::eIncreasing), a, a);
	EXPECT_FALSE(r.passed);
	EXPECT_EQ(r.failing, 1u);
}

TEST(Contract, SlopeIgnoresOffset) {
	auto e = make(4, 4);
	for (u32 i = 0; i < 16; ++i) {
		e.texels[i].x = static_cast<float>(i % 4) * 0.25F;
	}
	auto a = e;
	for (auto& t : a.texels) {
		t.x += 0.5F;
	}
	EXPECT_TRUE(run(slopeEpsilon(Axis::eX, 0, 1e-5F), a, e).passed);
	EXPECT_FALSE(run(absEpsilon(1e-5F), a, e).passed);
}

TEST(Contract, ListIsCapped) {
	const auto e = make(32, 32);
	const auto a = make(32, 32, 1.F);
	const auto r = run(absEpsilon(1e-3F), a, e);
	EXPECT_EQ(r.failing, 1024u);
	EXPECT_EQ(r.failingTexels.size(), 32u);
}

TEST(Contract, CustomKernel) {
	const auto a = make(1, 1);
	const Kernel k = [](const Raster&, const Raster&) { return CheckResult { "custom", false, 7, 2.F, {} }; };
	const auto r = check(Contract { { k } }, a, a);
	ASSERT_EQ(r.size(), 1u);
	EXPECT_EQ(r[0].name, "custom");
	EXPECT_EQ(r[0].failing, 7u);
}

TEST(Contract, ExtentMismatchFails) {
	const auto r = check(Contract { { absEpsilon(1e-3F) } }, make(4, 4), make(4, 2));
	ASSERT_EQ(r.size(), 1u);
	EXPECT_FALSE(r[0].passed);
	EXPECT_EQ(r[0].name, "extent");
}
