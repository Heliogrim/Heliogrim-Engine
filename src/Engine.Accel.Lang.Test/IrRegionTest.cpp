#include "pch.h"

#include <algorithm>

#include <Engine.Accel.Lang/Emitter/GlslEmitter.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Regions.hpp>
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

		/* no region parameter: those are specialized, not lowered in-function */
		[[nodiscard]] String lowerBody(StringView statements_) {
			return String {
					"@input (mode = bind, id = \"buf\", class = storage, access = read_write) block buf { u32[] data; };\n"
					"workgroup scratch: u32[256];\n"
					"fn main() {\nconst b: u32 = local_invocation_index * 8u;\n"
				} + String { statements_ } + "\n}\n";
		}

		/* GLSL text, or the first diagnostic */
		[[nodiscard]] String emitGlsl(ref<const Module> module_) {
			const auto layout = assignLayout(computeInterface(module_), module_.types, {});
			Language target {};
			const auto result = GlslEmitter {}.emit(module_, layout, target);
			if (not result.diagnostics.empty()) {
				return "error: " + result.diagnostics.front().message;
			}
			return target.text.empty() ? String {} : String { target.text.front().data };
		}

		[[nodiscard]] String emitLowered(ref<const Module> module_) {
			auto lowered = lowerRegions(module_);
			if (not lowered.has_value()) {
				return lowered.error().empty() ? String { "error" } : "error: " + lowered.error().front().message;
			}
			return emitGlsl(*lowered);
		}
	}

	TEST(RegionTest, LowerRewritesIndex) {
		auto module = parse(lowerBody("let r = &mut scratch[b .. b + 8u]; r[1u] = 5u;"));
		ASSERT_TRUE(module.has_value()) << module.error().front().message;
		const auto glsl = emitLowered(*module);
		EXPECT_TRUE(glsl.contains("scratch[")) << glsl;
		EXPECT_TRUE(glsl.contains("r__off + 1u")) << glsl;

		auto lowered = lowerRegions(*module);
		ASSERT_TRUE(lowered.has_value());
		EXPECT_FALSE(std::ranges::any_of(lowered->exprs, [](const auto& e_) { return e_.kind == ExprKind::eRange; }));
	}

	TEST(RegionTest, NestedSubRangeAbsoluteOffset) {
		auto module = parse(lowerBody("let l = &mut scratch[b .. b + 8u]; let v = &mut l[2u .. 4u]; v[1u] = 7u;"));
		ASSERT_TRUE(module.has_value()) << module.error().front().message;
		const auto glsl = emitLowered(*module);
		EXPECT_TRUE(glsl.contains("v__off = (l__off + 2u)")) << glsl;
		EXPECT_TRUE(glsl.contains("scratch[(v__off + 1u)]")) << glsl;
	}

	TEST(RegionTest, WholeReborrowKeepsLength) {
		auto module = parse(lowerBody("let a = &mut scratch[b + 4u .. b + 12u]; let b2 = &mut a; buf.data[0u] = len(b2);"));
		ASSERT_TRUE(module.has_value()) << module.error().front().message;
		const auto glsl = emitLowered(*module);
		EXPECT_TRUE(glsl.contains("b2__off = a__off")) << glsl;
		EXPECT_TRUE(glsl.contains("b2__len = a__len")) << glsl;
	}

	TEST(RegionTest, ElementAliasOfRegionWritesRoot) {
		auto module = parse(lowerBody("let r = &mut scratch[b + 4u .. b + 12u]; let e = &mut r[2u]; e = 5u;"));
		ASSERT_TRUE(module.has_value()) << module.error().front().message;
		const auto glsl = emitLowered(*module);
		EXPECT_TRUE(glsl.contains("_hg_ref0 = (r__off + 2u)")) << glsl;
		EXPECT_TRUE(glsl.contains("scratch[_hg_ref0] = 5u")) << glsl;
	}

	TEST(RegionTest, LenLowersToLength) {
		auto module = parse(lowerBody("let r = &mut scratch[b .. b + 8u]; let n: u32 = len(r); buf.data[0u] = n;"));
		ASSERT_TRUE(module.has_value()) << module.error().front().message;
		const auto glsl = emitLowered(*module);
		EXPECT_TRUE(glsl.contains("n = r__len")) << glsl;
	}

	[[nodiscard]] static String rowsBody(StringView statements_) {
		return String {
				"struct Inner { u32[16] d; }\n"
				"struct Outer { Inner[4] l; }\n"
				"workgroup s: Outer;\n"
				"fn main() {\nlet i: u32 = local_invocation_index;\n"
			} + String { statements_ } + "\n}\n";
	}

	TEST(RegionTest, RootIndexFrozenAtBorrow) {
		auto module = parse(rowsBody("let r = &mut s.l[i].d[0u .. 4u]; i = 3u; r[0u] = 1u;"));
		ASSERT_TRUE(module.has_value()) << module.error().front().message;
		const auto glsl = emitLowered(*module);
		EXPECT_TRUE(glsl.contains("r__i0 = i;")) << glsl;
		EXPECT_TRUE(glsl.contains("s.l[r__i0].d[(r__off + 0u)]")) << glsl;
		EXPECT_FALSE(glsl.contains("s.l[i]")) << glsl;
	}

	TEST(RegionTest, ShadowedIndexKeepsFrozenRoot) {
		auto module = parse(rowsBody("let r = &mut s.l[i].d[0u .. 4u]; { let i: u32 = 2u; r[0u] = i; }"));
		ASSERT_TRUE(module.has_value()) << module.error().front().message;
		const auto glsl = emitLowered(*module);
		EXPECT_TRUE(glsl.contains("s.l[r__i0].d[(r__off + 0u)] = i;")) << glsl;
	}

	TEST(RegionTest, ShadowedRootReported) {
		auto module = parse(lowerBody("let r = &mut scratch[b .. b + 8u]; { let scratch: u32 = 2u; r[0u] = scratch; }"));
		ASSERT_TRUE(module.has_value()) << module.error().front().message;
		EXPECT_EQ(emitLowered(*module), "error: 'scratch' shadows the root of live region 'r'");
	}

	TEST(RegionTest, LenReadsHoistedOffset) {
		auto module = parse(lowerBody("let r = &mut scratch[b .. b + 8u]; buf.data[0u] = len(r);"));
		ASSERT_TRUE(module.has_value()) << module.error().front().message;
		const auto glsl = emitLowered(*module);
		EXPECT_TRUE(glsl.contains("r__len = ((b + 8u) - r__off)")) << glsl;
	}

	TEST(RegionTest, ForInitRegionHoistedBeforeLoop) {
		auto module = parse(lowerBody("for (let r = &mut scratch[b .. b + 8u]; r[0u] < 4u; r[0u] += 1u) { r[1u] = 2u; }"));
		ASSERT_TRUE(module.has_value()) << module.error().front().message;
		const auto glsl = emitLowered(*module);
		const auto off = glsl.find("r__off = ");
		const auto loop = glsl.find("for (; ");
		ASSERT_NE(off, String::npos) << glsl;
		ASSERT_NE(loop, String::npos) << glsl;
		EXPECT_LT(off, loop) << glsl;
		EXPECT_LT(glsl.find("r__len = "), loop) << glsl;
	}

	TEST(RegionTest, EmitterRejectsUnloweredRegion) {
		auto module = parse(lowerBody("let r = &mut scratch[0u .. 8u]; r[1u] = 5u;"));
		ASSERT_TRUE(module.has_value()) << module.error().front().message;
		const auto glsl = emitGlsl(*module);
		EXPECT_TRUE(glsl.contains("region not lowered")) << glsl;
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

	TEST(RegionTest, ReservedLocalSuffixRejected) {
		EXPECT_EQ(errorOf(body("let r__off: u32 = 1u;")), "'r__off' ends in a suffix reserved for region lowering");
		EXPECT_EQ(errorOf(body("let a = &mut scratch[0u .. 8u]; let a__i0 = &mut a[0u .. 4u];")), "'a__i0' ends in a suffix reserved for region lowering");
		EXPECT_EQ(errorOf(body("let a__inner: u32 = 1u; let off__x: u32 = 2u;")), "");
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
