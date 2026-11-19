#include "../pch.h"

/**/

#include "../Harness/FunctionTarget.hpp"
#include "../Harness/FunctionTier.hpp"
#include "../Harness/GpuTest.hpp"

import std;

using namespace hg::test::gpu;
using namespace hg;

namespace {
	class FunctionTier :
		public GpuTest {};

	const InputsAt describe = [](const u32 x_, const u32 y_) { return "x=" + std::to_string(x_) + " y=" + std::to_string(y_); };
}

TEST_F(FunctionTier, TexelCoordinates) {

	const auto report = runFunction(
		device(),
		FunctionTestDesc {
			.name = "FunctionTier.TexelCoordinates",
			.fragmentAsl = R"(
@output (mode = forward, id = "color") f32vec4 color;

fn main() {
	color = f32vec4(frag_coord.x, frag_coord.y, 0.0, 1.0);
}
)",
			.extent = math::uivec2 { 33uL, 17uL },
			.expected = [](const u32 x_, const u32 y_) {
				return math::fvec4 { static_cast<f32>(x_) + 0.5F, static_cast<f32>(y_) + 0.5F, 0.F, 1.F };
			},
			.inputs = describe
		}
	);

	EXPECT_TRUE(report.passed()) << report.text();
}

TEST_F(FunctionTier, UnwrittenTargetFails) {

	// Writing the sentinel looks like no write at all; `discard` would need the demote-to-helper device feature.

	const auto report = runFunction(
		device(),
		FunctionTestDesc {
			.name = "FunctionTier.UnwrittenTargetFails",
			.fragmentAsl = R"(
@output (mode = forward, id = "color") f32vec4 color;

fn main() {
	color = f32vec4(-4096.0);
}
)",
			.expected = [](u32, u32) { return math::fvec4 { sentinel }; },
			.inputs = describe
		}
	);

	EXPECT_FALSE(report.passed());
	const auto ran = std::ranges::find(report.checks, String { "gpu-ran" }, &CheckResult::name);
	ASSERT_NE(ran, report.checks.end());
	EXPECT_FALSE(ran->passed);
}

TEST_F(FunctionTier, WrongExpectationReports) {

	const auto report = runFunction(
		device(),
		FunctionTestDesc {
			.name = "FunctionTier.WrongExpectationReports",
			.fragmentAsl = R"(
@output (mode = forward, id = "color") f32vec4 color;

fn main() {
	color = f32vec4(0.5);
}
)",
			.expected = [](u32, u32) { return math::fvec4 { 0.25F }; },
			.inputs = describe
		}
	);

	EXPECT_FALSE(report.passed());
	const auto abs = std::ranges::find_if(report.checks, [](const auto& c_) { return not c_.passed; });
	ASSERT_NE(abs, report.checks.end());
	EXPECT_EQ(abs->failing, 1024u);
	EXPECT_NE(report.text().find("(0, 0) x=0 y=0"), String::npos) << report.text();
}

TEST_F(FunctionTier, CompileErrorReports) {

	const auto report = runFunction(
		device(),
		FunctionTestDesc {
			.name = "FunctionTier.CompileErrorReports",
			.fragmentAsl = R"(
@output (mode = forward, id = "color") f32vec4 color;

fn main() {
	color = true;
}
)",
			.expected = [](u32, u32) { return math::fvec4 { 0.F }; },
			.inputs = describe
		}
	);

	EXPECT_FALSE(report.passed());
	EXPECT_NE(report.text().find("error[ASL0033]"), String::npos) << report.text();
}

TEST_F(FunctionTier, IncludeOutsideRootFails) {

	// Both name the repository's CMakeLists.txt, which exists, so only the root guard can reject them.
	const auto root = std::filesystem::path { HG_TEST_SHADER_ROOT };
	const auto outside = (root.parent_path().parent_path() / "CMakeLists.txt").generic_string();
	ASSERT_TRUE(std::filesystem::exists(outside)) << outside;

	for (const auto& include : { String { "../../CMakeLists.txt" }, outside }) {
		ASSERT_TRUE(std::filesystem::exists(root / include)) << include;

		const String fragment = "#include <" + include + ">\n"
			R"(@output (mode = forward, id = "color") f32vec4 color;

fn main() {
	color = f32vec4(0.0);
}
)";

		const auto report = runFunction(
			device(),
			FunctionTestDesc {
				.name = "FunctionTier.IncludeOutsideRootFails",
				.fragmentAsl = fragment,
				.extent = math::uivec2 { 4uL, 4uL },
				.expected = [](u32, u32) { return math::fvec4 { 0.F }; },
				.inputs = describe
			}
		);

		EXPECT_FALSE(report.passed()) << include;
		EXPECT_NE(report.text().find("unknown include '" + include + "'"), String::npos) << report.text();
	}
}
