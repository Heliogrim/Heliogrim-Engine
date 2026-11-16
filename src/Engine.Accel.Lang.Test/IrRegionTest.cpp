#include "pch.h"

#include <algorithm>
#include <regex>
#include "IrWording.hpp"

#include "IrSpirvText.hpp"
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/References.hpp>
#include <Engine.Accel.Lang/Ir/Variant.hpp>

using namespace hg::engine::accel::lang::ir;
using namespace hg::engine::accel::lang;
using namespace hg;

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
			if (not res.has_value() && not res.error().empty()) {
				AccelLang::expectWording(res.error().front());
			}
			return res.has_value() || res.error().empty() ? String {} : res.error().front().message();
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

		/* validated SPIR-V disassembly, or the first diagnostic */
		[[nodiscard]] String emitSpirv(ref<const Module> mod_) {
			const auto result = AccelLang::spirvOf(mod_);
			if (not result.diagnostics.empty()) {
				return "error: " + result.diagnostics.front().message();
			}
			return result.text;
		}

		[[nodiscard]] String emitLowered(ref<const Module> mod_) {
			auto lowered = lowerReferences(mod_);
			if (not lowered.has_value()) {
				return lowered.error().empty() ? String { "error" } : "error: " + lowered.error().front().message();
			}
			return emitSpirv(*lowered);
		}

		/* `pattern_` against the disassembly with the OpLine debug lines dropped, so adjacent instructions stay adjacent */
		[[nodiscard]] bool hasSequence(StringView text_, ptr<const char> pattern_) {
			const auto code = std::regex_replace(String { text_ }, std::regex { R"([ \t]*OpLine[^\n]*\n)" }, "");
			return std::regex_search(code, std::regex { pattern_ });
		}

		[[nodiscard]] std::ptrdiff_t countOf(StringView text_, StringView needle_) {
			auto count = std::ptrdiff_t {};
			for (auto at = text_.find(needle_); at != StringView::npos; at = text_.find(needle_, at + needle_.size())) {
				++count;
			}
			return count;
		}
	}

	TEST(RegionTest, LowerRewritesIndex) {
		auto mod = parse(lowerBody("let r = &mut scratch[b .. b + 8u]; r[1u] = 5u;"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitLowered(*mod);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %r_off\s+(%\d+) = OpIAdd %uint \1 %uint_1\s+(%\d+) = OpAccessChain %_ptr_Workgroup_uint %scratch \2\s+OpStore \3 %uint_5)")) << glsl;

		auto lowered = lowerReferences(*mod);
		ASSERT_TRUE(lowered.has_value());
		EXPECT_FALSE(std::ranges::any_of(lowered->exprs, [](const auto& e_) { return e_.kind == ExprKind::eRange; }));
	}

	TEST(RegionTest, NestedSubRangeAbsoluteOffset) {
		auto mod = parse(lowerBody("let l = &mut scratch[b .. b + 8u]; let v = &mut l[2u .. 4u]; v[1u] = 7u;"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitLowered(*mod);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %l_off\s+(%\d+) = OpIAdd %uint \1 %uint_2\s+OpStore %v_off \2)")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %v_off\s+(%\d+) = OpIAdd %uint \1 %uint_1\s+(%\d+) = OpAccessChain %_ptr_Workgroup_uint %scratch \2\s+OpStore \3 %uint_7)")) << glsl;
	}

	TEST(RegionTest, WholeReborrowKeepsLength) {
		auto mod = parse(lowerBody("let a = &mut scratch[b + 4u .. b + 12u]; let b2 = &mut a; buf.data[0u] = len(b2);"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitLowered(*mod);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %a_len\s+OpStore %b2_len \1)")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %b2_len\s+(%\d+) = OpAccessChain %_ptr_StorageBuffer_uint %buf %uint_0 %uint_0\s+OpStore \2 \1)")) << glsl;
	}

	TEST(RegionTest, ElementAliasOfRegionWritesRoot) {
		auto mod = parse(lowerBody("let r = &mut scratch[b + 4u .. b + 12u]; let e = &mut r[2u]; e = 5u;"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitLowered(*mod);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %r_off\s+(%\d+) = OpIAdd %uint \1 %uint_2\s+OpStore %e_i0 \2)")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %e_i0\s+(%\d+) = OpAccessChain %_ptr_Workgroup_uint %scratch \1\s+OpStore \2 %uint_5)")) << glsl;
	}

	TEST(RegionTest, LenLowersToLength) {
		auto mod = parse(lowerBody("let r = &mut scratch[b .. b + 8u]; let n: u32 = len(r); buf.data[0u] = n;"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitLowered(*mod);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %r_off\s+(%\d+) = OpISub %uint %\d+ \1\s+OpStore %r_len \2)")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %r_len\s+OpStore %n \1)")) << glsl;
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
		auto mod = parse(rowsBody("let r = &mut s.l[i].d[0u .. 4u]; i = 3u; r[0u] = 1u;"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitLowered(*mod);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %i\s+OpStore %r_i0 \1)")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"(OpStore %i %uint_3\s+(%\d+) = OpLoad %uint %r_i0\s+(%\d+) = OpLoad %uint %r_off\s+(%\d+) = OpIAdd %uint \2 %uint_0\s+(%\d+) = OpAccessChain %_ptr_Workgroup_uint %s %uint_0 \1 %uint_0 \3\s+OpStore \4 %uint_1)")) << glsl;
	}

	TEST(RegionTest, ShadowedIndexKeepsFrozenRoot) {
		auto mod = parse(rowsBody("let r = &mut s.l[i].d[0u .. 4u]; { let i: u32 = 2u; r[0u] = i; }"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitLowered(*mod);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"(OpStore %i_0 %uint_2[\s\S]*(%\d+) = OpLoad %uint %r_i0[\s\S]*= OpAccessChain %_ptr_Workgroup_uint %s %uint_0 \1 %uint_0 )")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %i_0\s+(%\d+) = OpAccessChain %_ptr_Workgroup_uint %s [^\n]*\s+OpStore \2 \1)")) << glsl;
	}

	TEST(RegionTest, ShadowedRootReported) {
		auto mod = parse(lowerBody("let r = &mut scratch[b .. b + 8u]; { let scratch: u32 = 2u; r[0u] = scratch; }"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		EXPECT_EQ(emitLowered(*mod), "error: 'scratch' shadows the root of live region 'r'");
	}

	TEST(RegionTest, LenReadsHoistedOffset) {
		auto mod = parse(lowerBody("let r = &mut scratch[b .. b + 8u]; buf.data[0u] = len(r);"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitLowered(*mod);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %r_off\s+(%\d+) = OpISub %uint %\d+ \1\s+OpStore %r_len \2)")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpLoad %uint %r_len\s+(%\d+) = OpAccessChain %_ptr_StorageBuffer_uint %buf %uint_0 %uint_0\s+OpStore \2 \1)")) << glsl;
	}

	TEST(RegionTest, ForInitRegionHoistedBeforeLoop) {
		auto mod = parse(lowerBody("for (let r = &mut scratch[b .. b + 8u]; r[0u] < 4u; r[0u] += 1u) { r[1u] = 2u; }"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitLowered(*mod);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_EQ(countOf(glsl, "OpStore %r_off "), 1);
		EXPECT_EQ(countOf(glsl, "OpStore %r_len "), 1);
		EXPECT_LT(glsl.find("OpStore %r_len "), glsl.find("OpLoopMerge"));
	}

	TEST(RegionTest, EmitterRejectsUnloweredRegion) {
		auto mod = parse(lowerBody("let r = &mut scratch[0u .. 8u]; r[1u] = 5u;"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto result = spirv::emit(*mod, assignLayout(computeInterface(*mod), mod->types, {}), Tier::eDev);
		ASSERT_FALSE(result.diagnostics.empty());
		EXPECT_EQ(result.diagnostics.front().code()->key, "internal-spirv-emit");
		EXPECT_TRUE(result.words.empty());
	}

	TEST(RegionTest, HelperSpecializedOncePerRoot) {
		auto mod = parse(body("w(scratch[0u .. 8u], 0u); w(scratch[8u .. 16u], 1u); w(buf.data[0u .. 4u], 2u);"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		auto lowered = lowerReferences(*mod);
		ASSERT_TRUE(lowered.has_value()) << lowered.error().front().message();
				const auto specs = std::ranges::count_if(lowered->functions, [](const auto& fn_) { return fn_.name.empty() && fn_.origin == "w"; });
		EXPECT_EQ(specs, 2);
		EXPECT_FALSE(std::ranges::any_of(lowered->functions, [](const auto& fn_) { return fn_.name == "w"; }));
		const auto glsl = emitSpirv(*lowered);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_EQ(countOf(glsl, " = OpFunction "), 3);
		EXPECT_TRUE(hasSequence(glsl, R"(OpFunctionCall %void (%\d+) %uint_0 %uint_8 %uint_0\s+%\d+ = OpFunctionCall %void \1 %uint_8 %uint_16 %uint_1\s+%\d+ = OpFunctionCall %void (?!\1 )%\d+ %uint_0 %uint_4 %uint_2)")) << glsl;
	}

	TEST(RegionTest, HelperRangeLowerBoundEmittedOnce) {
		auto mod = parse(body("w(scratch[local_invocation_index * 4u .. 64u], 0u);"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitLowered(*mod);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_EQ(countOf(glsl, " = OpIMul "), 1);
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpIMul %uint %\d+ %uint_4\s+%\d+ = OpFunctionCall %void %\d+ \1 %uint_64 %uint_0)")) << glsl;
	}

	TEST(RegionTest, TransitiveSpecialization) {
		auto mod = parse(
			String { prelude } + "fn inner(v: &mut u32[]) { v[0u] = 1u; }\nfn outer(v: &mut u32[]) { inner(v[1u .. 3u]); }\n"
			"fn main() { outer(scratch[0u .. 8u]); }\n"
		);
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitLowered(*mod);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_EQ(countOf(glsl, " = OpFunction "), 3);
		EXPECT_FALSE(glsl.contains("\"inner\"") || glsl.contains("\"outer\"")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"(= OpFunctionCall %void %\d+ %uint_0 %uint_8\s)")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpIAdd %uint %\d+ %uint_1\s+[\s\S]*?(%\d+) = OpIAdd %uint %\d+ %uint_3\s+%\d+ = OpFunctionCall %void %\d+ \1 \2)")) << glsl;
	}

	TEST(RegionTest, LocalArrayRootRejected) {
		auto mod = parse(body("let tmp: u32[4]; w(tmp[0u .. 4u], 0u);"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto message = emitLowered(*mod);
		EXPECT_TRUE(message.contains("region argument to 'w' must be rooted in a buffer or workgroup array")) << message;
	}

	TEST(RegionTest, HelperLocalCapturingRootReported) {
		auto mod = parse(
			String { prelude } + "fn h(v: &mut u32[]) { let scratch: u32 = 2u; v[0u] = scratch; }\nfn main() { h(scratch[0u .. 8u]); }\n"
		);
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		EXPECT_EQ(emitLowered(*mod), "error: 'scratch' shadows the root of live region 'v'");
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
		EXPECT_TRUE(message.contains("range bounds must be 'u32'")) << message;
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

	TEST(RegionTest, PlainHelperAfterRegionHelperKeepsCalls) {
		auto m = parse(String { prelude } + "fn h(v: u32) { scratch[v] = 9u; }\nfn main() { w(scratch[0u .. 8u], 0u); h(3u); }\n");
		ASSERT_TRUE(m.has_value());
		const auto glsl = emitLowered(*m);
		ASSERT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_EQ(countOf(glsl, " = OpFunction "), 3);
		EXPECT_TRUE(hasSequence(glsl, R"(= OpFunctionCall %void %h %uint_3)")) << glsl;
		EXPECT_TRUE(hasSequence(glsl, R"((%\d+) = OpAccessChain %_ptr_Workgroup_uint %scratch %\d+\s+OpStore \1 %uint_9)")) << glsl;
	}

	TEST(RegionTest, RegionSuffixIsALocalName) {
		for (const auto& source : {
				 body("let r__off: u32 = 1u; scratch[r__off] = 2u;"), body("let a = &mut scratch[0u .. 8u]; let a__i0 = &mut a[0u .. 4u]; a__i0[1u] = 3u;"),
				 body("let a__inner: u32 = 1u; let off__x: u32 = 2u; scratch[a__inner] = off__x;")
			 }) {
			ASSERT_EQ(errorOf(source), "") << source;
			const auto spirv = emitLowered(*parse(source));
			EXPECT_FALSE(spirv.starts_with("error:")) << source << "\n" << spirv;
		}
	}

	TEST(RegionTest, RegionSuffixIsAParamAndFunctionName) {
		for (const auto& source : {
				 String { prelude } + "fn g(p__end: u32) { scratch[p__end] = 1u; }\nfn main() { g(1u); }\n",
				 String { prelude } + "fn g__r0() { scratch[0u] = 1u; }\nfn main() { g__r0(); }\n",
				 String { prelude } + "fn g__ring(p__lens: u32) { scratch[p__lens] = 1u; }\nfn main() { g__ring(1u); }\n"
			 }) {
			ASSERT_EQ(errorOf(source), "") << source;
			const auto spirv = emitLowered(*parse(source));
			EXPECT_FALSE(spirv.starts_with("error:")) << source << "\n" << spirv;
		}
	}

	TEST(RegionTest, UserParamWithDoubleUnderscoreKeepsItsName) {
		const auto source = String { prelude } + "fn g(a__b: u32) { scratch[a__b] = 1u; }\nfn main() { g(1u); }\n";
		const auto spirv = emitLowered(*parse(source));
		ASSERT_FALSE(spirv.starts_with("error:")) << spirv;
		EXPECT_TRUE(spirv.contains("OpName %a__b \"a__b\"")) << spirv;
	}

	TEST(RegionTest, UserLocalNamedLikeGeneratedLength) {
		auto mod = parse(lowerBody("let r = &mut scratch[b .. b + 8u]; let r__len: u32 = 3u; buf.data[0u] = len(r) + r__len;"));
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto spirv = emitLowered(*mod);
		ASSERT_FALSE(spirv.starts_with("error:")) << spirv;
		EXPECT_TRUE(spirv.contains("OpName %r_len \"r$len\"")) << spirv;
		EXPECT_TRUE(hasSequence(spirv, R"((%\d+) = OpLoad %uint %r_len\s+(%\d+) = OpLoad %uint %r__len\s+(%\d+) = OpIAdd %uint \1 \2)")) << spirv;
	}

	TEST(RegionTest, UserParamNamedLikeGeneratedIndex) {
		const auto source = String { prelude } + "fn put(v: &mut u32, v__i0: u32) { v = v__i0; }\nfn main() { let i: u32 = local_invocation_index; put(buf.data[i], 5u); }\n";
		const auto spirv = emitLowered(*parse(source));
		ASSERT_FALSE(spirv.starts_with("error:")) << spirv;
		EXPECT_TRUE(hasSequence(spirv, R"(= OpFunctionCall %void %\d+ %\d+ %uint_5)")) << spirv;
		// the index parameter (first) addresses the buffer, the user parameter (second) is stored
		EXPECT_TRUE(
			hasSequence(
				spirv,
				R"((%\d+) = OpFunctionParameter %uint\s+(%\d+) = OpFunctionParameter %uint\s+%\d+ = OpLabel\s+(%\d+) = OpVariable [^\n]*\s+(%\d+) = OpVariable [^\n]*\s+OpStore \3 \1\s+OpStore \4 \2\s+(%\d+) = OpLoad %uint \3\s+(%\d+) = OpLoad %uint \4\s+(%\d+) = OpAccessChain %_ptr_StorageBuffer_uint %buf %uint_0 \5\s+OpStore \7 \6)"
			)
		) << spirv;
	}

	TEST(RegionTest, WholeArrayArgumentPassesItsLength) {
		const auto source = String { prelude } + "fn n(v: &mut u32[]) { buf.data[0u] = len(v); }\nfn main() { n(scratch); }\n";
		const auto spirv = emitLowered(*parse(source));
		ASSERT_FALSE(spirv.starts_with("error:")) << spirv;
		EXPECT_TRUE(hasSequence(spirv, R"(= OpFunctionCall %void %\w+ %uint_0 %uint_256)")) << spirv;
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
