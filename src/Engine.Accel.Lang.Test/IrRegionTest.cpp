#include "pch.h"

#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Variant.hpp>

using namespace hg;
using namespace hg::engine::accel::lang;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] std::expected<Module, Diagnostics> parse(StringView source_) {
			return parseModule("M", source_, EntryKind::eCompute, ModuleResolver { noIncludes });
		}

		[[nodiscard]] String errorOf(StringView source_) {
			auto res = parse(source_);
			return res.has_value() || res.error().empty() ? String {} : res.error().front().message;
		}

		constexpr auto prelude =
			"@input (mode = bind, id = \"buf\", class = storage, access = read_write) block buf { u32[] data; };\n"
			"workgroup scratch: u32[256];\n"
			"fn w(v: &mut u32[], i: u32) { v[i] = 1u; }\n";

		[[nodiscard]] String body(StringView statements_) {
			return String { prelude } + "fn main() {\n" + String { statements_ } + "\n}\n";
		}
	}

	TEST(RegionTest, RangeBorrowTypechecks) {
		EXPECT_EQ(errorOf(body("let r = &mut scratch[0u .. 64u]; r[3u] = 1u; let n: u32 = len(r);")), "");
	}

	TEST(RegionTest, SubRangeOfRegion) {
		EXPECT_EQ(errorOf(body("let r = &mut scratch[0u .. 64u]; let s = &mut r[8u .. 16u]; s[0u] = 2u;")), "");
	}

	TEST(RegionTest, RangeOnUnsizedArray) {
		EXPECT_EQ(errorOf(body("let r = &mut buf.data[4u .. 12u];")), "");
	}

	TEST(RegionTest, RegionParam) {
		EXPECT_EQ(errorOf(body("w(scratch[0u .. 8u], 0u);")), "");
	}

	TEST(RegionTest, SizedArrayPassesAsRegion) {
		EXPECT_EQ(errorOf(body("w(scratch, 0u);")), "");
		const auto message = errorOf(body("w(buf.data, 0u);"));
		EXPECT_TRUE(message.contains("unsized array needs an explicit range")) << message;
	}

	TEST(RegionTest, RangeOutsideBorrowIsDiagnostic) {
		const auto message = errorOf(body("let x = scratch[0u .. 4u];"));
		EXPECT_TRUE(message.contains("a range needs a borrow")) << message;
	}

	TEST(RegionTest, ConstantRangeReversed) {
		const auto message = errorOf(body("let r = &mut scratch[8u .. 4u];"));
		EXPECT_TRUE(message.contains("empty or reversed range")) << message;
	}

	TEST(RegionTest, RangeBoundsAreU32) {
		const auto message = errorOf(body("let r = &mut scratch[0.0 .. 4.0];"));
		EXPECT_TRUE(message.contains("range bounds must be u32")) << message;
	}

	TEST(RegionTest, RangeOnNonArray) {
		const auto message = errorOf(body("let x: u32 = 0u; let r = &mut x[0u .. 1u];"));
		EXPECT_TRUE(message.contains("cannot range")) << message;
	}

	TEST(RegionTest, RangeIsHashed) {
		auto a = parse(body("let r = &mut scratch[0u .. 8u];"));
		auto b = parse(body("let r = &mut scratch[0u .. 9u];"));
		ASSERT_TRUE(a.has_value());
		ASSERT_TRUE(b.has_value());
		EXPECT_NE(contentHash(*a), contentHash(*b));
	}

	TEST(RegionTest, OverloadsByRegionElement) {
		const String src = String { prelude } +
			"fn w(v: &mut f32[], i: u32) { v[i] = 1.0; }\n"
			"workgroup fs: f32[16];\n"
			"fn main() { w(scratch[0u .. 8u], 0u); w(fs[0u .. 4u], 0u); }\n";
		EXPECT_EQ(errorOf(src), "");
		const auto message = errorOf(body("let b: bool = true; w(b, 0u);"));
		EXPECT_FALSE(message.contains("sampler")) << message;
	}

	TEST(RegionTest, UserFunctionShadowsLen) {
		const String src = String { prelude } + "fn len(x: u32) -> u32 { return x; }\nfn main() { let n: u32 = len(3u); }\n";
		EXPECT_EQ(errorOf(src), "");
	}

	TEST(RegionTest, SplitLaneIntoIndexAndVertex) {
		EXPECT_EQ(
			errorOf(
				body(
					"const base: u32 = local_invocation_index * 16u; let lane = &mut scratch[base .. base + 16u]; const n: u32 = 6u; "
					"let idx = &mut lane[0u .. n]; let vtx = &mut lane[n .. 16u]; idx[0u] = 1u; vtx[0u] = 2u;"
				)
			),
			""
		);
	}

	TEST(RegionTest, OverlappingLetBorrowsRejected) {
		const auto message = errorOf(body("let a = &mut scratch[0u .. 8u]; let b = &mut scratch[4u .. 12u];"));
		EXPECT_TRUE(message.contains("overlaps live borrow")) << message;
	}

	TEST(RegionTest, ReadAndMutOverlapRejected) {
		const auto message = errorOf(body("let a = &scratch[0u .. 8u]; let b = &mut scratch[0u .. 2u];"));
		EXPECT_TRUE(message.contains("overlaps live borrow")) << message;
	}

	TEST(RegionTest, TwoReadsOverlapAllowed) {
		EXPECT_EQ(errorOf(body("let a = &scratch[0u .. 8u]; let b = &scratch[0u .. 8u];")), "");
	}

	TEST(RegionTest, BorrowEndsWithScope) {
		EXPECT_EQ(errorOf(body("{ let a = &mut scratch[0u .. 8u]; } let b = &mut scratch[0u .. 8u];")), "");
	}

	TEST(RegionTest, MutableSharedBoundRejected) {
		const auto message = errorOf(body("let n: u32 = 4u; let a = &mut scratch[0u .. n]; n = 8u; let b = &mut scratch[n .. 16u];"));
		EXPECT_TRUE(message.contains("overlaps live borrow")) << message;
	}

	TEST(RegionTest, ReadOnlyRegionWriteRejected) {
		EXPECT_NE(errorOf(body("let r = &scratch[0u .. 8u]; r[0u] = 1u;")), "");
		EXPECT_NE(errorOf(String { prelude } + "fn g(r: &u32[]) { r[0u] = 1u; }\nfn main() { g(scratch[0u .. 8u]); }\n"), "");
	}

	TEST(RegionTest, LetOverlapNamesBothPlaces) {
		const auto msg = errorOf(body("let a = &mut scratch[0u .. 8u]; let b = &mut scratch[4u .. 12u];"));
		EXPECT_TRUE(msg.contains("scratch[0u .. 8u]")) << msg;
		EXPECT_TRUE(msg.contains("scratch[4u .. 12u]")) << msg;
	}

	TEST(RegionTest, SiblingReborrowRejected) {
		EXPECT_NE(errorOf(body("let a = &mut scratch[0u .. 8u]; let b = &mut a[0u .. 4u]; let c = &mut a[2u .. 6u];")), "");
	}

	TEST(RegionTest, ParentExemptionDoesNotCoverLiveChild) {
		EXPECT_NE(errorOf(body("let a = &mut scratch[0u .. 8u]; let b = &mut a[0u .. 4u]; w(a[0u .. 2u], 0u);")), "");
	}

	TEST(RegionTest, ArgumentOverlappingLiveLetRejected) {
		EXPECT_EQ(
			errorOf(body("let a = &mut scratch[0u .. 8u]; w(scratch[0u .. 8u], 0u);")),
			"'&mut' argument 'scratch[0u .. 8u]' overlaps live borrow 'scratch[0u .. 8u]' of call to 'w'"
		);
	}

	TEST(RegionTest, ArgumentThroughLiveAliasAllowed) {
		EXPECT_EQ(errorOf(body("let a = &mut scratch[0u .. 8u]; w(a, 0u); w(a[0u .. 4u], 1u);")), "");
	}

	TEST(RegionTest, ArgumentDisjointFromLiveLetAllowed) {
		EXPECT_EQ(errorOf(body("let a = &mut scratch[0u .. 8u]; w(scratch[8u .. 16u], 0u);")), "");
	}
}
