#include "../pch.h"

/**/

#include <Engine.Common/__macro.hpp>

#include "../Harness/Contract.hpp"
#include "../Harness/FunctionTier.hpp"
#include "../Harness/GpuTest.hpp"

import Engine.Common;

using namespace hg::test::gpu;
using namespace hg;

namespace {
	class Pbr :
		public GpuTest {};

	// Enough texels per axis to resolve the shape of each lobe; matches the 64-step input sweeps.
	inline constexpr static u32 side = 64uL;
	inline constexpr static math::uivec2 extent { side, side };

	// Lowest swept roughness; keeps the GGX lobe resolvable on a 64-texel raster and the epsilon guard inactive.
	inline constexpr static f64 minRough = 0.05;

	// Dielectric base reflectance used throughout the shaders.
	inline constexpr static f64 dielectricF0 = 0.04;

	// Fixed roughness for the shadowing sweep, a mid value where k = 0.125 bends the curve visibly.
	inline constexpr static f64 geoRough = 0.5;

	// Fixed cosines for the diffuse sweep, away from both grazing and normal incidence.
	inline constexpr static f64 diffuseNdv = 0.5;
	inline constexpr static f64 diffuseLdh = 0.5;

	inline constexpr static f64 pi = std::numbers::pi_v<f64>;

	[[nodiscard]] f64 centre(u32 i_);

	[[nodiscard]] f64 roughAt(u32 i_);

	[[nodiscard]] math::fvec4 scalar(f64 value_);

	[[nodiscard]] f64 ggx(f64 ndh_, f64 rough_);

	[[nodiscard]] f64 smith(f64 ndl_, f64 ndv_, f64 rough_);

	[[nodiscard]] f64 schlick(f64 ct_, f64 f0_, f64 f90_);

	[[nodiscard]] f64 burley(f64 ndv_, f64 ndl_, f64 ldh_, f64 rough_);

	[[nodiscard]] std::string pair(ptr<const char> xName_, f64 x_, ptr<const char> yName_, f64 y_);

	[[nodiscard]] Contract fresnelContract();

	const InputsAt describeFresnel = [](const u32 x_, u32) { return "ct=" + std::to_string(centre(x_)) + " f0=0.04 f90=1"; };

	constexpr auto fresnelExpected = [](const u32 x_, u32) { return scalar(schlick(centre(x_), dielectricF0, 1.0)); };
}

TEST_F(Pbr, NdfGgx) {

	const auto report = runFunction(
		device(),
		FunctionTestDesc {
			.name = "Pbr.NdfGgx",
			.fragmentAsl = R"(
#include <shared/math.asl>
#include <shared/pbr.asl>

@output (mode = forward, id = "color") f32vec4 color;

fn main() {
	let rough = 0.05 + 0.95 * frag_coord.x / 64.0;
	let ndh = frag_coord.y / 64.0;
	color = f32vec4(ndfGgx(ndh, rough), 0.0, 0.0, 1.0);
}
)",
			.extent = extent,
			.expected = [](const u32 x_, const u32 y_) { return scalar(ggx(centre(y_), roughAt(x_))); },
			.inputs = [](const u32 x_, const u32 y_) { return pair("rough", roughAt(x_), "ndh", centre(y_)); },
			.contract = Contract { { relEpsilon(1e-4F, 1), monotonic(Axis::eY, 0, Order::eIncreasing) } }
		}
	);

	EXPECT_TRUE(report.passed()) << report.text();
}

TEST_F(Pbr, GeoSchlickSmith) {

	const auto report = runFunction(
		device(),
		FunctionTestDesc {
			.name = "Pbr.GeoSchlickSmith",
			.fragmentAsl = R"(
#include <shared/math.asl>
#include <shared/pbr.asl>

@output (mode = forward, id = "color") f32vec4 color;

fn main() {
	let ndl = frag_coord.x / 64.0;
	let ndv = frag_coord.y / 64.0;
	color = f32vec4(geoSchlickSmith(ndl, ndv, 0.5), 0.0, 0.0, 1.0);
}
)",
			.extent = extent,
			.expected = [](const u32 x_, const u32 y_) { return scalar(smith(centre(x_), centre(y_), geoRough)); },
			.inputs = [](const u32 x_, const u32 y_) { return pair("ndl", centre(x_), "ndv", centre(y_)) + " rough=0.5"; },
			.contract = Contract { { absEpsilon(defaultAbsEpsilon, 1), monotonic(Axis::eX, 0, Order::eIncreasing) } }
		}
	);

	EXPECT_TRUE(report.passed()) << report.text();
}

TEST_F(Pbr, FresnelSchlickScalar) {

	const auto report = runFunction(
		device(),
		FunctionTestDesc {
			.name = "Pbr.FresnelSchlickScalar",
			.fragmentAsl = R"(
#include <shared/math.asl>
#include <shared/pbr.asl>

@output (mode = forward, id = "color") f32vec4 color;

fn main() {
	let ct = frag_coord.x / 64.0;
	color = f32vec4(fresnelSchlick(ct, 0.04, 1.0), 0.0, 0.0, 1.0);
}
)",
			.extent = extent,
			.expected = fresnelExpected,
			.inputs = describeFresnel,
			.contract = fresnelContract()
		}
	);

	EXPECT_TRUE(report.passed()) << report.text();
}

TEST_F(Pbr, DisneyDiffuse) {

	const auto report = runFunction(
		device(),
		FunctionTestDesc {
			.name = "Pbr.DisneyDiffuse",
			.fragmentAsl = R"(
#include <shared/math.asl>
#include <shared/pbr.asl>

@output (mode = forward, id = "color") f32vec4 color;

fn main() {
	let ndl = frag_coord.x / 64.0;
	let rough = frag_coord.y / 64.0;
	color = f32vec4(disneyDiffuse(0.5, ndl, 0.5, rough), 0.0, 0.0, 1.0);
}
)",
			.extent = extent,
			.expected = [](const u32 x_, const u32 y_) { return scalar(burley(diffuseNdv, centre(x_), diffuseLdh, centre(y_))); },
			.inputs = [](const u32 x_, const u32 y_) { return pair("ndl", centre(x_), "rough", centre(y_)) + " ndv=0.5 ldh=0.5"; },
			.contract = Contract { { absEpsilon(defaultAbsEpsilon, 1) } }
		}
	);

	EXPECT_TRUE(report.passed()) << report.text();
}

TEST_F(Pbr, ChangedFunctionFails) {

	// A local copy of the scalar fresnelSchlick with exponent 4 instead of 5.

	const auto report = runFunction(
		device(),
		FunctionTestDesc {
			.name = "Pbr.ChangedFunctionFails",
			.fragmentAsl = R"(
@output (mode = forward, id = "color") f32vec4 color;

fn changedFresnel(ct: f32, f0: f32, f90: f32) -> f32 {
	return f0 + (f90 - f0) * pow(1.0 - ct, 4.0);
}

fn main() {
	let ct = frag_coord.x / 64.0;
	color = f32vec4(changedFresnel(ct, 0.04, 1.0), 0.0, 0.0, 1.0);
}
)",
			.extent = extent,
			.expected = fresnelExpected,
			.inputs = describeFresnel,
			.contract = fresnelContract()
		}
	);

	EXPECT_FALSE(report.passed());
	const auto abs = std::ranges::find(report.checks, String { "absEpsilon" }, &CheckResult::name);
	ASSERT_NE(abs, report.checks.end());
	EXPECT_FALSE(abs->passed);
	EXPECT_FALSE(abs->failingTexels.empty());
	EXPECT_NE(report.text().find("(0, 0) ct=0.007812 f0=0.04 f90=1"), String::npos) << report.text();
}

/**/

namespace {
	[[nodiscard]] f64 centre(const u32 i_) {
		return (static_cast<f64>(i_) + 0.5) / static_cast<f64>(side);
	}

	[[nodiscard]] f64 roughAt(const u32 i_) {
		return minRough + (1.0 - minRough) * centre(i_);
	}

	[[nodiscard]] math::fvec4 scalar(const f64 value_) {
		return math::fvec4 { static_cast<f32>(value_), 0.F, 0.F, 1.F };
	}

	// Walter et al. 2007, eq. 33 with alpha = rough^2 (Burley 2012 remapping, as the shader states).
	[[nodiscard]] f64 ggx(const f64 ndh_, const f64 rough_) {
		const auto alpha2 = std::pow(rough_, 4.0);
		const auto cos2 = ndh_ * ndh_;
		const auto tan2 = (1.0 - cos2) / cos2;
		const auto term = alpha2 + tan2;
		return ndh_ <= 0.0 ? 0.0 : alpha2 / (pi * cos2 * cos2 * term * term);
	}

	// Schlick 1994 G1 = v / (v (1 - k) + k), Smith separable product; k = rough^2 / 2 as the shader states.
	[[nodiscard]] f64 smith(const f64 ndl_, const f64 ndv_, const f64 rough_) {
		const auto k = rough_ * rough_ / 2.0;
		const auto g1 = [k](const f64 v_) { return v_ / (v_ * (1.0 - k) + k); };
		return g1(ndl_) * g1(ndv_);
	}

	// Schlick 1994, eq. 15 generalised to a grazing reflectance f90.
	[[nodiscard]] f64 schlick(const f64 ct_, const f64 f0_, const f64 f90_) {
		return f0_ + (f90_ - f0_) * std::pow(1.0 - ct_, 5.0);
	}

	// Burley 2012 diffuse with the Frostbite energy renormalisation (Lagarde & de Rousiers 2014, listing 1), times 1 / PI.
	[[nodiscard]] f64 burley(const f64 ndv_, const f64 ndl_, const f64 ldh_, const f64 rough_) {
		const auto energyBias = 0.5 * rough_;
		const auto energyFactor = 1.0 + (1.0 / 1.51 - 1.0) * rough_;
		const auto fd90 = energyBias + 2.0 * ldh_ * ldh_ * rough_;
		return schlick(ndl_, 1.0, fd90) * schlick(ndv_, 1.0, fd90) * energyFactor / pi;
	}

	[[nodiscard]] std::string pair(const ptr<const char> xName_, const f64 x_, const ptr<const char> yName_, const f64 y_) {
		return std::string { xName_ } + "=" + std::to_string(x_) + " " + yName_ + "=" + std::to_string(y_);
	}

	[[nodiscard]] Contract fresnelContract() {
		return Contract {
			{
				absEpsilon(defaultAbsEpsilon, 1),
				monotonic(Axis::eX, 0, Order::eDecreasing),
				slopeEpsilon(Axis::eX, 0, 1e-4F)
			}
		};
	}
}
