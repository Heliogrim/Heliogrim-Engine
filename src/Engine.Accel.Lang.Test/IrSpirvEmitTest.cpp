#include "pch.h"

/**/

#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/References.hpp>
#include <Engine.Accel.Lang/Ir/Tier.hpp>
#include <Engine.Accel.Lang/Ir/Spirv/SpirvEmitter.hpp>
#include <spirv-tools/libspirv.hpp>

using namespace hg::engine::accel::lang::ir;
using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	Opt<String> noIncludes(StringView) {
		return None;
	}

	[[nodiscard]] String disasm(ref<const Vector<u32>> words_) {
		spvtools::SpirvTools tools { SPV_ENV_VULKAN_1_3 };
		std::string text {};
		if (not tools.Disassemble(words_, &text, SPV_BINARY_TO_TEXT_OPTION_NO_HEADER | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES)) {
			return "<disassembly failed>";
		}
		return String { text };
	}

	struct Emitted {
		spirv::EmitResult result;
		String text;
	};

	struct Prepared {
		Module mod;
		Layout layout;
	};

	/* parse, lower references and lay out */
	[[nodiscard]] Opt<Prepared> prepare(StringView src_, EntryKind entry_) {
		auto parsed = parseModule("M", src_, entry_, ModuleResolver { noIncludes });
		EXPECT_TRUE(parsed.has_value()) << (parsed.has_value() || parsed.error().empty() ? "" : parsed.error().front().message());
		if (not parsed.has_value()) {
			return None;
		}
		auto lowered = lowerReferences(*parsed);
		EXPECT_TRUE(lowered.has_value());
		if (not lowered.has_value()) {
			return None;
		}
		auto layout = assignLayout(computeInterface(*lowered), lowered->types, {});
		layout.specConstants = assignSpecConstants(lowered->constants);
		return Prepared { std::move(*lowered), std::move(layout) };
	}

	/* emit a prepared module; fails the test on any diagnostic */
	[[nodiscard]] Emitted emitPrepared(ref<const Prepared> prepared_, Tier tier_ = Tier::eDev) {
		auto result = spirv::emit(prepared_.mod, prepared_.layout, tier_);
		for (const auto& diagnostic : result.diagnostics) {
			ADD_FAILURE() << diagnostic.message();
		}
		EXPECT_FALSE(result.words.empty());
		auto text = disasm(result.words);
		return Emitted { std::move(result), std::move(text) };
	}

	/* parse, lower references, lay out and emit; fails the test on any diagnostic */
	[[nodiscard]] Emitted emitOf(StringView src_, EntryKind entry_, Tier tier_ = Tier::eDev) {
		const auto prepared = prepare(src_, entry_);
		return prepared.has_value() ? emitPrepared(*prepared, tier_) : Emitted {};
	}

	[[nodiscard]] bool has(ref<const String> text_, StringView needle_) {
		return text_.find(needle_) != String::npos;
	}
}

TEST(IrSpirvEmitTest, UniformBlockOffsetsStd140) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"u\", class = uniform) block u { f32 a; f32vec3 b; f32[4] c; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { f32 x = u.a; }\n",
		EntryKind::eCompute
	);
	EXPECT_TRUE(has(out.text, " 0 Offset 0")) << out.text;
	EXPECT_TRUE(has(out.text, " 1 Offset 16")) << out.text;
	EXPECT_TRUE(has(out.text, " 2 Offset 32")) << out.text;
	EXPECT_TRUE(has(out.text, "ArrayStride 16")) << out.text;
	EXPECT_TRUE(has(out.text, "Uniform")) << out.text;
}

TEST(IrSpirvEmitTest, StorageBlockOffsetsStd430) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"u\", class = storage) block u { f32 a; f32vec3 b; f32[4] c; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { f32 x = u.a; }\n",
		EntryKind::eCompute
	);
	EXPECT_TRUE(has(out.text, " 1 Offset 16")) << out.text;
	EXPECT_TRUE(has(out.text, " 2 Offset 28")) << out.text;
	EXPECT_TRUE(has(out.text, "ArrayStride 4")) << out.text;
	EXPECT_TRUE(has(out.text, "StorageBuffer")) << out.text;
}

TEST(IrSpirvEmitTest, BoolMemberIsUint) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"cfg\", class = uniform) block cfg { bool enabled; };\n"
		"@output (mode = forward, id = \"color\") f32vec4 color;\n"
		"@fragment\nfn main() { if (cfg.enabled) { color = f32vec4(1.0, 1.0, 1.0, 1.0); } }\n",
		EntryKind::eFragment
	);
	EXPECT_TRUE(has(out.text, "OpTypeStruct %uint")) << out.text;
	EXPECT_FALSE(has(out.text, "OpTypeStruct %bool")) << out.text;
}

TEST(IrSpirvEmitTest, DescriptorDecorations) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"u\", class = uniform) block u { f32 y; };\n"
		"@input (mode = bind, id = \"t\", class = texture) texture2d t;\n"
		"@input (mode = bind, id = \"s\", class = sampler) sampler s;\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { f32 x = u.y; }\n",
		EntryKind::eCompute
	);
	EXPECT_TRUE(has(out.text, "DescriptorSet 0")) << out.text;
	EXPECT_TRUE(has(out.text, "Binding 0")) << out.text;
	EXPECT_TRUE(has(out.text, " Block")) << out.text;
}

TEST(IrSpirvEmitTest, ComputeLocalSize) {
	const auto out = emitOf("@compute (local_size = [64, 1, 1])\nfn main() {}\n", EntryKind::eCompute);
	EXPECT_TRUE(has(out.text, "OpExecutionMode %main LocalSize 64 1 1")) << out.text;
}

TEST(IrSpirvEmitTest, SpecConstantId) {
	const auto out = emitOf(
		"@spec (id = \"a\") const A: u32 = 1u;\n@spec (id = \"b\") const B: u32 = 2u;\n"
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { u32 v; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { o.v = B; }\n",
		EntryKind::eCompute
	);
	EXPECT_TRUE(has(out.text, "SpecId 1")) << out.text;
	EXPECT_TRUE(has(out.text, "OpSpecConstant %uint 2")) << out.text;
}

TEST(IrSpirvEmitTest, NoNamesInBuildTier) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"u\", class = uniform) block u { f32 y; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { f32 x = u.y; }\n",
		EntryKind::eCompute,
		Tier::eBuild
	);
	EXPECT_FALSE(has(out.text, "OpName")) << out.text;
	EXPECT_FALSE(has(out.text, "OpMemberName")) << out.text;
}

TEST(IrSpirvEmitTest, EmptyVertex) {
	const auto out = emitOf("fn main() {}\n", EntryKind::eVertex);
	EXPECT_TRUE(has(out.text, "OpEntryPoint Vertex %main \"main\"")) << out.text;
}

TEST(IrSpirvEmitTest, EmptyTessCtrl) {
	const auto out = emitOf("@tess_ctrl (output_vertices = 3)\nfn main() {}\n", EntryKind::eTessCtrl);
	EXPECT_TRUE(has(out.text, "OpExecutionMode %main OutputVertices 3")) << out.text;
	EXPECT_TRUE(has(out.text, "OpCapability Tessellation")) << out.text;
}

TEST(IrSpirvEmitTest, EmptyTessEval) {
	const auto out = emitOf(
		"@tess_eval (domain = triangles, spacing = equal, winding = cw)\nfn main() {}\n",
		EntryKind::eTessEval
	);
	EXPECT_TRUE(has(out.text, "OpExecutionMode %main Triangles")) << out.text;
	EXPECT_TRUE(has(out.text, "OpExecutionMode %main SpacingEqual")) << out.text;
	EXPECT_TRUE(has(out.text, "OpExecutionMode %main VertexOrderCw")) << out.text;
}

TEST(IrSpirvEmitTest, EmptyGeometry) {
	const auto out = emitOf(
		"@geometry (input = triangles, output = triangle_strip, max_vertices = 3)\nfn main() {}\n",
		EntryKind::eGeometry
	);
	EXPECT_TRUE(has(out.text, "OpExecutionMode %main OutputVertices 3")) << out.text;
	EXPECT_TRUE(has(out.text, "OpExecutionMode %main OutputTriangleStrip")) << out.text;
	EXPECT_TRUE(has(out.text, "OpCapability Geometry")) << out.text;
}

TEST(IrSpirvEmitTest, EmptyFragment) {
	const auto out = emitOf("@fragment\nfn main() {}\n", EntryKind::eFragment);
	EXPECT_TRUE(has(out.text, "OpExecutionMode %main OriginUpperLeft")) << out.text;
}

TEST(IrSpirvEmitTest, EmptyCompute) {
	const auto out = emitOf("@compute (local_size = [1, 1, 1])\nfn main() {}\n", EntryKind::eCompute);
	EXPECT_TRUE(has(out.text, "OpEntryPoint GLCompute %main \"main\"")) << out.text;
}

TEST(IrSpirvEmitTest, OutputStorageStaysWritable) {
	const auto out = emitOf(
		"@output (mode = bind, id = \"o\", class = storage) block o { f32 x; };\n"
		"@input (mode = bind, id = \"i\", class = storage, access = read) block i { f32 y; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { o.x = i.y; }\n",
		EntryKind::eCompute
	);
	EXPECT_FALSE(has(out.text, "OpDecorate %o NonWritable")) << out.text;
	EXPECT_TRUE(has(out.text, "OpDecorate %i NonWritable")) << out.text;
}

TEST(IrSpirvEmitTest, StorageImageFormats) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"a\", class = image, access = write) image2d<rgba16f> a;\n"
		"@input (mode = bind, id = \"b\", class = image, access = write) image2d<rgba8> b;\n"
		"@input (mode = bind, id = \"c\", class = image, access = write) image2d<r32f> c;\n"
		"@input (mode = bind, id = \"d\", class = image, access = write) image2d<r32ui> d;\n"
		"@compute (local_size = [1, 1, 1])\nfn main() {\n"
		"\timage_store(a, s32vec2(0, 0), f32vec4(1.0));\n\timage_store(b, s32vec2(0, 0), f32vec4(1.0));\n"
		"\timage_store(c, s32vec2(0, 0), f32vec4(1.0));\n\timage_store(d, s32vec2(0, 0), u32vec4(1u));\n}\n",
		EntryKind::eCompute
	);
	EXPECT_TRUE(has(out.text, "OpTypeImage %float 2D 0 0 0 2 Rgba16f")) << out.text;
	EXPECT_TRUE(has(out.text, "OpTypeImage %float 2D 0 0 0 2 Rgba8")) << out.text;
	EXPECT_TRUE(has(out.text, "OpTypeImage %float 2D 0 0 0 2 R32f")) << out.text;
	EXPECT_TRUE(has(out.text, "OpTypeImage %uint 2D 0 0 0 2 R32ui")) << out.text;
	EXPECT_FALSE(has(out.text, " 2 Unknown")) << out.text;
}

TEST(IrSpirvEmitTest, PushMembersAtAbsoluteOffsets) {
	auto prepared = prepare(
		"@input (mode = bind, id = \"p\", class = push) block p { f32 a; f32vec4 b; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { f32 x = p.a; }\n",
		EntryKind::eCompute
	);
	ASSERT_TRUE(prepared.has_value());
	ASSERT_EQ(prepared->layout.push.size(), 1uz);
	// as if another stage's range sat before it: GLSL puts `a` at 4 and `b` at its absolute alignment 16
	prepared->layout.push[0].offset = 4uL;
	const auto out = emitPrepared(*prepared);
	EXPECT_TRUE(has(out.text, " 0 Offset 4")) << out.text;
	EXPECT_TRUE(has(out.text, " 1 Offset 16")) << out.text;
}

TEST(IrSpirvEmitTest, TwoPushBlocksAreDiagnosed) {
	const auto prepared = prepare(
		"@input (mode = bind, id = \"p\", class = push) block p { f32 a; };\n"
		"@input (mode = bind, id = \"q\", class = push) block q { f32 b; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { f32 x = p.a + q.b; }\n",
		EntryKind::eCompute
	);
	ASSERT_TRUE(prepared.has_value());
	const auto result = spirv::emit(prepared->mod, prepared->layout, Tier::eDev);
	ASSERT_EQ(result.diagnostics.size(), 1uz);
	EXPECT_EQ(result.diagnostics[0].code()->key, "too-many-push-constant-blocks");
}

TEST(IrSpirvEmitTest, IntegerFragmentInputsAreFlat) {
	const auto out = emitOf(
		"@input (mode = forward, id = \"ki\") s32 ki;\n"
		"@input (mode = forward, id = \"ka\") u32[2] ka;\n"
		"@input (mode = forward, id = \"kf\") f32 kf;\n"
		"@output (mode = forward, id = \"oi\") u32 oi;\n"
		"@output (mode = forward, id = \"of\") f32 of;\n"
		"@fragment\nfn main() { oi = ka[1]; of = kf; s32 x = ki; }\n",
		EntryKind::eFragment
	);
	EXPECT_TRUE(has(out.text, "OpDecorate %ki Flat")) << out.text;
	EXPECT_TRUE(has(out.text, "OpDecorate %ka Flat")) << out.text;
	EXPECT_FALSE(has(out.text, "OpDecorate %kf Flat")) << out.text;
}

TEST(IrSpirvEmitTest, UniformStd430IsDiagnosedAtMain) {
	const auto prepared = prepare(
		"@input (mode = bind, id = \"u\", class = uniform, packing = std430) block u { f32 a; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { f32 x = u.a; }\n",
		EntryKind::eCompute
	);
	ASSERT_TRUE(prepared.has_value());
	const auto result = spirv::emit(prepared->mod, prepared->layout, Tier::eDev);
	ASSERT_EQ(result.diagnostics.size(), 1uz);
	EXPECT_EQ(result.diagnostics[0].code()->key, "internal-spirv-emit");
	const auto main = std::ranges::find(prepared->mod.functions, String { "main" }, &FunctionDecl::name);
	ASSERT_NE(main, prepared->mod.functions.end());
	const auto& span = result.diagnostics[0].primary().as<engine::diag::SourceSpan>();
	EXPECT_EQ(span.line, main->loc.line);
	EXPECT_EQ(span.column, main->loc.column);
	EXPECT_NE(span.line, 1uL);
}

TEST(IrSpirvEmitTest, DevNamesOnlyUserParamsAndSpecConstants) {
	const auto out = emitOf(
		"@spec (id = \"flag\") const FLAG: bool = true;\n"
		"const SCALE: f32 = 2.0;\n"
		"@output (mode = forward, id = \"color\") f32 color;\n"
		"fn G(userParam: f32) -> f32 { return userParam * SCALE; }\n"
		"fn F(x: &mut f32, refParam: f32) { x = refParam; }\n"
		"fn main() { let a = G(1.0); F(a, 2.0); if (FLAG) { color = a; } }\n",
		EntryKind::eFragment
	);
	EXPECT_TRUE(has(out.text, "\"userParam\"")) << out.text;
	EXPECT_FALSE(has(out.text, "\"refParam\"")) << out.text;
	EXPECT_FALSE(has(out.text, "\"SCALE\"")) << out.text;
	EXPECT_TRUE(has(out.text, "\"FLAG\"")) << out.text;
	EXPECT_TRUE(has(out.text, "OpSpecConstantTrue")) << out.text;
}

namespace {
	/* text of the function `name_` from its OpFunction to its OpFunctionEnd */
	[[nodiscard]] String functionText(ref<const String> text_, StringView name_) {
		const auto begin = text_.find(String { "%" } + String { name_ } + " = OpFunction ");
		if (begin == String::npos) {
			return {};
		}
		return text_.substr(begin, text_.find("OpFunctionEnd", begin) - begin);
	}

	/* text of the block labelled `label_` up to the next label */
	[[nodiscard]] String blockText(ref<const String> text_, StringView label_) {
		const auto begin = text_.find(String { label_ } + " = OpLabel");
		if (begin == String::npos) {
			return {};
		}
		const auto end = text_.find("OpLabel", begin + label_.size() + 9uz);
		return text_.substr(begin, end == String::npos ? String::npos : end - begin);
	}

	[[nodiscard]] size_t count(ref<const String> text_, StringView needle_) {
		size_t n = 0uz;
		for (auto at = text_.find(needle_); at != String::npos; at = text_.find(needle_, at + 1uz)) {
			++n;
		}
		return n;
	}
}

TEST(IrSpirvEmitTest, ContinueInNestedIfRunsStep) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { u32 v; };\n"
		"@compute (local_size = [1, 1, 1])\n"
		"fn main() { for (let i = 0u; i < 4u; i += 1u) { if (i == o.v) { continue; } o.v = o.v * i; } }\n",
		EntryKind::eCompute
	);
	const auto merge = out.text.find("OpLoopMerge ");
	ASSERT_NE(merge, String::npos) << out.text;
	const auto operands = out.text.substr(merge + 12uz, out.text.find('\n', merge) - merge - 12uz);
	const auto space = operands.find(' ');
	ASSERT_NE(space, String::npos) << out.text;
	const auto cont = operands.substr(space + 1uz, operands.find(' ', space + 1uz) - space - 1uz);
	const auto block = blockText(out.text, cont);
	EXPECT_TRUE(has(block, "OpIAdd")) << cont << "\n" << out.text;
	EXPECT_EQ(count(out.text, "OpIAdd"), 1uz) << out.text;
}

TEST(IrSpirvEmitTest, EarlyReturnInLoop) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { f32 x; u32 n; };\n"
		"fn find(n: u32) -> f32 {\n\tlet i = 0u;\n\twhile (i < n) {\n\t\tif (i == 3u) { return 1.0; }\n\t\ti += 1u;\n\t}\n"
		"\twhile (i < 8u) { return 2.0; }\n\treturn 0.0;\n}\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { o.x = find(o.n); }\n",
		EntryKind::eCompute
	);
	EXPECT_EQ(count(functionText(out.text, "find"), "OpLoopMerge"), 2uz) << out.text;
	EXPECT_TRUE(has(functionText(out.text, "find"), "OpReturnValue")) << out.text;
}

TEST(IrSpirvEmitTest, ShortCircuitWithCall) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { f32 x; u32 n; };\n"
		"fn f() -> bool { o.n = 1u; return true; }\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { if (o.n > 0u && f()) { o.x = 1.0; } }\n",
		EntryKind::eCompute
	);
	const auto main = functionText(out.text, "main");
	const auto call = main.find("OpFunctionCall");
	ASSERT_NE(call, String::npos) << out.text;
	EXPECT_LT(main.find("OpBranchConditional"), call) << out.text;
	EXPECT_TRUE(has(main, "OpPhi")) << out.text;
}

TEST(IrSpirvEmitTest, TernaryPureUsesSelect) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { f32 x; f32 a; f32 b; u32 n; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { let c = o.n > 1u; let x = o.a; let y = o.b; o.x = c ? x : y; }\n",
		EntryKind::eCompute
	);
	EXPECT_TRUE(has(out.text, "OpSelect")) << out.text;
	EXPECT_FALSE(has(out.text, "OpPhi")) << out.text;
}

TEST(IrSpirvEmitTest, TernaryIntegerDivisionBranches) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { u32 x; u32 a; u32 n; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { let n = o.n; o.x = n != 0u ? o.a / n : 0u; }\n",
		EntryKind::eCompute
	);
	EXPECT_FALSE(has(out.text, "OpSelect ")) << out.text;
	EXPECT_TRUE(has(out.text, "OpPhi")) << out.text;
}

TEST(IrSpirvEmitTest, TernaryDynamicIndexBranches) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { f32 x; u32 i; f32[4] v; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { let i = o.i; o.x = i < 4u ? o.v[i] : 0.0; }\n",
		EntryKind::eCompute
	);
	EXPECT_FALSE(has(out.text, "OpSelect ")) << out.text;
	EXPECT_TRUE(has(out.text, "OpPhi")) << out.text;
}

TEST(IrSpirvEmitTest, TernaryConstantIndexInRangeSelects) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { f32 x; u32 i; f32[4] v; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { let i = o.i; o.x = i < 4u ? o.v[2u] : 0.0; }\n",
		EntryKind::eCompute
	);
	EXPECT_TRUE(has(out.text, "OpSelect ")) << out.text;
	EXPECT_FALSE(has(out.text, "OpPhi")) << out.text;
}

TEST(IrSpirvEmitTest, TernaryConstantIndexOutOfRangeBranches) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { f32 x; u32 i; f32[4] v; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { let i = o.i; o.x = i < 4u ? o.v[7u] : 0.0; }\n",
		EntryKind::eCompute
	);
	EXPECT_FALSE(has(out.text, "OpSelect ")) << out.text;
	EXPECT_TRUE(has(out.text, "OpPhi")) << out.text;
}

TEST(IrSpirvEmitTest, DiscardTerminates) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"u\", class = uniform) block u { f32 k; };\n"
		"@output (mode = forward, id = \"color\") f32vec4 color;\n"
		"fn kill() { discard; }\n"
		"@fragment\nfn main() {\n\tif (u.k > 1.0) { discard; }\n\tif (u.k > 2.0) { kill(); }\n\tcolor = f32vec4(u.k);\n}\n",
		EntryKind::eFragment
	);
	EXPECT_EQ(count(out.text, "OpTerminateInvocation"), 2uz) << out.text;
	EXPECT_TRUE(has(functionText(out.text, "kill"), "OpTerminateInvocation")) << out.text;
}

TEST(IrSpirvEmitTest, SignedDivision) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { s32 a; s32 b; u32 c; u32 d; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { o.a = o.a / o.b; o.c = o.c / o.d; }\n",
		EntryKind::eCompute
	);
	EXPECT_TRUE(has(out.text, "OpSDiv")) << out.text;
	EXPECT_TRUE(has(out.text, "OpUDiv")) << out.text;
}

TEST(IrSpirvEmitTest, ConversionOpcodes) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { s32 i; f32 f; f32 x; u32 y; u32 z; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { o.x = f32(o.i); o.y = u32(o.f); o.z = bitcast<u32>(o.f); }\n",
		EntryKind::eCompute
	);
	EXPECT_TRUE(has(out.text, "OpConvertSToF")) << out.text;
	EXPECT_TRUE(has(out.text, "OpConvertFToU")) << out.text;
	EXPECT_TRUE(has(out.text, "OpBitcast")) << out.text;
}

TEST(IrSpirvEmitTest, LocalReferenceParameter) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { u32 k; };\n"
		"struct P { u32 colour; u32 w; }\n"
		"fn paint(c: &mut u32) { c = 3u; }\n"
		"fn b(w: &mut u32) { w += 1u; }\n"
		"fn a(v: &mut P) { b(v.colour); }\n"
		"@compute (local_size = [1, 1, 1])\n"
		"fn main() {\nlet i: u32 = o.k;\nlet arr: P[4];\npaint(arr[i].colour);\na(arr[i]);\no.k = arr[i].colour;\n}\n",
		EntryKind::eCompute
	);
	EXPECT_EQ(count(out.text, "OpFunctionCall"), 3uz) << out.text;
	EXPECT_TRUE(has(out.text, "OpTypePointer Function %_arr_P_uint_4")) << out.text;
}

namespace {
	/* spirv-val under vulkan 1.3; messages go to `log_` */
	[[nodiscard]] bool valid(ref<const Vector<u32>> words_, ref<String> log_) {
		spvtools::SpirvTools tools { SPV_ENV_VULKAN_1_3 };
		tools.SetMessageConsumer([&](spv_message_level_t, const char*, const spv_position_t&, const char* message_) { log_ += message_; });
		return tools.Validate(words_.data(), words_.size());
	}

	/* the label id operand at `index_` (0 merge, 1 continue) of the `nth_` OpLoopMerge */
	[[nodiscard]] String loopLabel(ref<const String> text_, size_t nth_, size_t index_) {
		auto at = text_.find("OpLoopMerge ");
		for (size_t i = 0uz; i < nth_ && at != String::npos; ++i) {
			at = text_.find("OpLoopMerge ", at + 1uz);
		}
		if (at == String::npos) {
			return {};
		}
		auto begin = at + 12uz;
		for (size_t i = 0uz; i < index_; ++i) {
			begin = text_.find(' ', begin) + 1uz;
		}
		return text_.substr(begin, text_.find_first_of(" \n", begin) - begin);
	}
}

TEST(IrSpirvEmitTest, BoolMemberCondition) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"cfg\", class = uniform) block cfg { bool enabled; };\n"
		"@output (mode = forward, id = \"color\") f32vec4 color;\n"
		"@fragment\nfn main() { if (cfg.enabled) { color = f32vec4(1.0, 1.0, 1.0, 1.0); } }\n",
		EntryKind::eFragment
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "OpINotEqual")) << out.text;
}

TEST(IrSpirvEmitTest, SwizzleStoreEvaluatesRhsFirst) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { f32vec4 v; };\n"
		"fn make(v: &mut f32vec4) -> f32vec2 { v = f32vec4(9.0); return f32vec2(1.0, 2.0); }\n"
		"@compute (local_size = [1, 1, 1])\n"
		"fn main() { let v = o.v; v.xy = make(v); v.zw += f32vec2(1.0, 1.0); o.v = v; }\n",
		EntryKind::eCompute
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	const auto main = functionText(out.text, "main");
	const auto call = main.find("OpFunctionCall");
	const auto shuffle = main.find("OpVectorShuffle");
	ASSERT_NE(call, String::npos) << out.text;
	ASSERT_NE(shuffle, String::npos) << out.text;
	// the vector merged into is reloaded after the call wrote it back
	const auto reload = main.find("OpLoad %v4float %v\n", call);
	EXPECT_LT(reload, shuffle) << out.text;
	EXPECT_LT(call, shuffle) << out.text;
}

TEST(IrSpirvEmitTest, CompoundAssignEvaluatesRhsFirst) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { f32 x; };\n"
		"fn bump(x: &mut f32) -> f32 { x = 9.0; return 1.0; }\n"
		"@compute (local_size = [1, 1, 1])\n"
		"fn main() { let x = o.x; x += bump(x); o.x = x; }\n",
		EntryKind::eCompute
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	const auto main = functionText(out.text, "main");
	const auto call = main.find("OpFunctionCall");
	const auto add = main.find("OpFAdd");
	ASSERT_NE(call, String::npos) << out.text;
	ASSERT_NE(add, String::npos) << out.text;
	// the target is loaded after the call wrote it
	EXPECT_LT(main.find("OpLoad %float %x\n", call), add) << out.text;
}

TEST(IrSpirvEmitTest, AtomicOnNonPlaceIsInternalDiagnostic) {
	auto prepared = prepare(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { u32 x; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { let old = atomic_add(o.x, 1u); }\n",
		EntryKind::eCompute
	);
	ASSERT_TRUE(prepared.has_value());
	// sema rejects a non-place; corrupt the checked module to reach the emitter guard
	auto atomic = std::ranges::find_if(prepared->mod.exprs, [](const auto& e_) { return e_.name == "atomic_add"; });
	ASSERT_NE(atomic, prepared->mod.exprs.end());
	atomic->operands[0] = atomic->operands[1];
	const auto result = spirv::emit(prepared->mod, prepared->layout, Tier::eDev);
	EXPECT_TRUE(result.words.empty());
	ASSERT_FALSE(result.diagnostics.empty());
	EXPECT_TRUE(
		result.diagnostics.front().message().ends_with("atomic on a non-addressable operand")
	) << result.diagnostics.front().message();
}

TEST(IrSpirvEmitTest, CompoundAssignIndexEvaluatedOnce) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { u32[4] a; u32 n; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { o.a[o.n] += 1u; }\n",
		EntryKind::eCompute
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	const auto main = functionText(out.text, "main");
	// one load of the index o.n, one of a[i]; both access chains reuse the index id
	EXPECT_EQ(count(main, "OpAccessChain"), 3uz) << out.text;
	EXPECT_EQ(count(main, "OpLoad %uint"), 2uz) << out.text;
}

TEST(IrSpirvEmitTest, MatrixScalarArithmetic) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { f32mat2 m; f32 k; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { o.m = o.m + o.k; o.m = o.k - o.m; o.m = o.m / o.k; }\n",
		EntryKind::eCompute
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_EQ(count(out.text, "OpFAdd %v2float"), 2uz) << out.text;
	EXPECT_EQ(count(out.text, "OpFSub %v2float"), 2uz) << out.text;
	EXPECT_EQ(count(out.text, "OpFDiv %v2float"), 2uz) << out.text;
}

TEST(IrSpirvEmitTest, NestedLoopBreakContinueTargetInnerLoop) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { u32 n; };\n"
		"@compute (local_size = [1, 1, 1])\n"
		"fn main() {\n\tfor (let i = 0u; i < 4u; i += 1u) {\n\t\tfor (let j = 0u; j < 4u; j += 1u) {\n"
		"\t\t\tif (j == o.n) { break; }\n\t\t\tif (j == i) { continue; }\n\t\t\to.n += j;\n\t\t}\n\t}\n}\n",
		EntryKind::eCompute
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	const auto merge = loopLabel(out.text, 1uz, 0uz);
	const auto cont = loopLabel(out.text, 1uz, 1uz);
	ASSERT_FALSE(merge.empty()) << out.text;
	EXPECT_TRUE(has(out.text, "OpBranch " + merge + "\n")) << merge << "\n" << out.text;
	EXPECT_GE(count(out.text, "OpBranch " + cont + "\n"), 2uz) << cont << "\n" << out.text;
}

TEST(IrSpirvEmitTest, TextureSampleValidates) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"t\", class = texture) texture2d t;\n"
		"@input (mode = bind, id = \"s\", class = sampler) sampler s;\n"
		"fn main() {\n\tlet c = sample(t, s, f32vec2(0.5));\n}\n",
		EntryKind::eFragment
	);
	EXPECT_TRUE(has(out.text, "OpSampledImage")) << out.text;
	EXPECT_TRUE(has(out.text, "OpImageSampleImplicitLod")) << out.text;
}

TEST(IrSpirvEmitTest, Int64DeclaresCapability) {
	const auto out = emitOf("fn main() {\n\tlet x: u64 = 1;\n}\n", EntryKind::eFragment);
	EXPECT_TRUE(has(out.text, "OpCapability Int64")) << out.text;
}

TEST(IrSpirvEmitTest, DevTierHasLines) {
	const auto src = "fn main() {\n\tlet a = 1.0;\n\tlet b = a + 2.0;\n}\n";
	const auto dev = emitOf(src, EntryKind::eFragment);
	EXPECT_TRUE(has(dev.text, "OpString")) << dev.text;
	EXPECT_TRUE(has(dev.text, "OpSource Unknown 0")) << dev.text;
	EXPECT_TRUE(has(dev.text, "OpLine %1 3 ")) << dev.text;
	const auto build = emitOf(src, EntryKind::eFragment, Tier::eBuild);
	EXPECT_FALSE(has(build.text, "OpLine")) << build.text;
	EXPECT_FALSE(has(build.text, "OpString")) << build.text;
}

TEST(IrSpirvEmitTest, DerivativeAndBarrier) {
	const auto frag = emitOf("fn main() {\n\tlet d = dFdx(frag_coord.x);\n}\n", EntryKind::eFragment);
	EXPECT_TRUE(has(frag.text, "OpDPdx")) << frag.text;
	EXPECT_TRUE(has(frag.text, "BuiltIn FragCoord")) << frag.text;
	const auto comp = emitOf("@compute (local_size = [1, 1, 1])\nfn main() {\n\tbarrier();\n}\n", EntryKind::eCompute);
	EXPECT_TRUE(has(comp.text, "OpControlBarrier")) << comp.text;
}

TEST(IrSpirvEmitTest, SampleOutsideFragmentIsExplicitLod) {
	const auto bindings = "@input (mode = bind, id = \"t\", class = texture) texture2d t;\n"
		"@input (mode = bind, id = \"s\", class = sampler) sampler s;\n";
	const auto vert = emitOf(String { bindings } + "fn main() {\n\tlet c = sample(t, s, f32vec2(0.5));\n}\n", EntryKind::eVertex);
	const auto comp = emitOf(
		String { bindings } + "@compute (local_size = [1, 1, 1])\nfn main() {\n\tlet c = sample(t, s, f32vec2(0.5));\n}\n",
		EntryKind::eCompute
	);
	for (const auto* out : { &vert, &comp }) {
		String log {};
		EXPECT_TRUE(valid(out->result.words, log)) << log << "\n" << out->text;
		EXPECT_TRUE(has(out->text, "OpImageSampleExplicitLod")) << out->text;
		EXPECT_FALSE(has(out->text, "ImplicitLod")) << out->text;
	}
}

TEST(IrSpirvEmitTest, StorageImageExtendedFormats) {
	// the parser only spells rgba8/rgba16f/r32f/r32ui; retag the r32f type to reach the emitter's extended formats
	for (const auto* format : { "rg32f", "rg16f", "r16f" }) {
		auto prepared = prepare(
			"@input (mode = bind, id = \"a\", class = image, access = write) image2d<r32f> a;\n"
			"@compute (local_size = [1, 1, 1])\nfn main() {\n\timage_store(a, s32vec2(0, 0), f32vec4(1.0));\n}\n",
			EntryKind::eCompute
		);
		ASSERT_TRUE(prepared.has_value());
		auto image = std::ranges::find(prepared->mod.types, String { "r32f" }, &TypeDesc::name);
		ASSERT_NE(image, prepared->mod.types.end());
		image->name = format;
		const auto out = emitPrepared(*prepared);
		String log {};
		EXPECT_TRUE(valid(out.result.words, log)) << format << "\n" << log << "\n" << out.text;
		EXPECT_TRUE(has(out.text, "OpCapability StorageImageExtendedFormats")) << out.text;
	}
}

TEST(IrSpirvEmitTest, AtomicsOnBufferAndWorkgroup) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { u32 n; };\n"
		"workgroup counter: u32;\n"
		"@compute (local_size = [1, 1, 1])\nfn main() {\n"
		"\tlet a = atomic_add(o.n, 1u);\n\tlet b = atomic_comp_swap(o.n, a, 2u);\n"
		"\tlet c = atomic_add(counter, 1u);\n\tlet d = atomic_comp_swap(counter, c, 3u);\n}\n",
		EntryKind::eCompute
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_EQ(count(out.text, "OpAtomicIAdd"), 2uz) << out.text;
	EXPECT_EQ(count(out.text, "OpAtomicCompareExchange"), 2uz) << out.text;
}

TEST(IrSpirvEmitTest, ImageLoadScalarResult) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"a\", class = image, access = read) image2d<r32f> a;\n"
		"@compute (local_size = [1, 1, 1])\nfn main() {\n\tlet v = image_load(a, s32vec2(0, 0));\n}\n",
		EntryKind::eCompute
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "OpImageRead")) << out.text;
}

TEST(IrSpirvEmitTest, TextureSizeIncludingCube) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"t\", class = texture) texture2d t;\n"
		"@input (mode = bind, id = \"u\", class = texture) texturecube u;\n"
		"@input (mode = bind, id = \"s\", class = sampler) sampler s;\n"
		"fn main() {\n\tlet a = texture_size(t, s, 0);\n\tlet b = texture_size(u, s, 0);\n}\n",
		EntryKind::eFragment
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_EQ(count(out.text, "OpImageQuerySizeLod"), 2uz) << out.text;
}

TEST(IrSpirvEmitTest, MemoryBarrierShared) {
	const auto out = emitOf("@compute (local_size = [1, 1, 1])\nfn main() {\n\tmemory_barrier_shared();\n}\n", EntryKind::eCompute);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "OpMemoryBarrier")) << out.text;
}

TEST(IrSpirvEmitTest, FragDepthDeclaresDepthReplacing) {
	const auto out = emitOf("fn main() {\n\tfrag_depth = 0.5;\n}\n", EntryKind::eFragment);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "DepthReplacing")) << out.text;
}

TEST(IrSpirvEmitTest, TessCtrlPerVertexBuiltins) {
	const auto out = emitOf(
		"@input (mode = forward, id = \"world-pos\") f32vec4[] p;\n"
		"@output (mode = forward, id = \"ctrl-pos\") f32vec4[] q;\n"
		"@output (mode = forward, id = \"level\", patch = true) f32 lvl;\n"
		"@tess_ctrl (output_vertices = 3)\n"
		"fn main() {\n\tq[invocation_id] = p[invocation_id];\n"
		"\toutput_position[invocation_id] = input_position[invocation_id];\n"
		"\tlvl = 2.0;\n\ttess_level_outer[0] = 2.0;\n\ttess_level_inner[0] = 2.0;\n}\n",
		EntryKind::eTessCtrl
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "BuiltIn TessLevelOuter")) << out.text;
	EXPECT_TRUE(has(out.text, "Patch")) << out.text;
}

TEST(IrSpirvEmitTest, GeometryPerVertexBuiltins) {
	const auto out = emitOf(
		"@geometry (input = triangles, output = triangle_strip, max_vertices = 3)\n"
		"fn main() {\n\tposition = input_position[0];\n\temit_vertex();\n\tend_primitive();\n}\n",
		EntryKind::eGeometry
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "OpEmitVertex")) << out.text;
}

TEST(IrSpirvEmitTest, MissingMainGetsEmptyEntry) {
	const auto out = emitOf("fn helper() {\n}\n", EntryKind::eFragment);
	String log {};
	EXPECT_TRUE(out.result.diagnostics.empty());
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "OpEntryPoint Fragment")) << out.text;
}

/* emit without failing on diagnostics; returns the joined diagnostic messages */
static String emitDiagnostics(StringView src_, EntryKind entry_) {
	const auto prepared = prepare(src_, entry_);
	if (not prepared.has_value()) {
		return {};
	}
	String joined {};
	for (const auto& diagnostic : spirv::emit(prepared->mod, prepared->layout, Tier::eDev).diagnostics) {
		joined += String { diagnostic.message() } + "\n";
	}
	return joined;
}

TEST(IrSpirvEmitTest, VertexWithoutMainIsDiagnostic) {
	const auto diagnostics = emitDiagnostics("fn helper() {\n}\n", EntryKind::eVertex);
	EXPECT_TRUE(has(diagnostics, "no 'main'")) << diagnostics;
}

TEST(IrSpirvEmitTest, UnsignedFoldWrapsAtDeclaredWidth) {
	const auto out = emitOf("const H: u32 = (0u - 1u) / 2u;\nfn main() {\n\tposition = f32vec4(f32(H));\n}\n", EntryKind::eVertex);
	EXPECT_TRUE(has(out.text, "OpConstant %uint 2147483647\n")) << out.text;
}

TEST(IrSpirvEmitTest, SignedFoldWrapsAtDeclaredWidth) {
	const auto out = emitOf("const H: s32 = (2147483647 + 1) / 2;\nfn main() {\n\tposition = f32vec4(f32(H));\n}\n", EntryKind::eVertex);
	EXPECT_TRUE(has(out.text, "OpConstant %int -1073741824\n")) << out.text;
}

TEST(IrSpirvEmitTest, FoldDivisionByZeroIsDiagnostic) {
	const auto diagnostics = emitDiagnostics(
		"const H: u32 = 1u / (1u - 1u);\nfn main() {\n\tposition = f32vec4(f32(H));\n}\n",
		EntryKind::eVertex
	);
	EXPECT_TRUE(has(diagnostics, "division by zero")) << diagnostics;
}

TEST(IrSpirvEmitTest, FoldMostNegativeByMinusOneIsDiagnostic) {
	const auto diagnostics = emitDiagnostics(
		"const H: s32 = (-2147483647 - 1) / -1;\nfn main() {\n\tposition = f32vec4(f32(H));\n}\n",
		EntryKind::eVertex
	);
	EXPECT_TRUE(has(diagnostics, "most negative")) << diagnostics;
}

TEST(IrSpirvEmitTest, FoldConvertsArrayElementsToElementType) {
	const auto out = emitOf(
		"const W: f32[2] = f32[2](1, 0.5);\nfn main() {\n\tposition = f32vec4(W[0]);\n}\n",
		EntryKind::eVertex
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "OpConstant %float 1\n")) << out.text;
	EXPECT_FALSE(has(out.text, "OpConstant %int 1\n")) << out.text;
}

TEST(IrSpirvEmitTest, CompositeModuleConstantFolds) {
	const auto out = emitOf(
		"const POSITIONS: f32vec2[3] = f32vec2[3](f32vec2(0.0, 1.0), f32vec2(1.0), f32vec2(-1.0, 0.0));\n"
		"fn main() {\n\tposition = f32vec4(POSITIONS[1], 0.0, 1.0);\n}\n",
		EntryKind::eVertex
	);
	String log {};
	EXPECT_TRUE(out.result.diagnostics.empty());
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "OpConstantComposite")) << out.text;
}

TEST(IrSpirvEmitTest, FoldConvertsVectorComponentsToComponentType) {
	const auto out = emitOf(
		"const V: f32vec2 = f32vec2(u32vec2(1u, 2u));\nconst W: f32vec2 = f32vec2(1, 2);\n"
		"fn main() {\n\tposition = f32vec4(V + W, 0.0, 1.0);\n}\n",
		EntryKind::eVertex
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "OpConstant %float 2\n")) << out.text;
}

TEST(IrSpirvEmitTest, MatrixConstantFromMatrixConstantFolds) {
	const auto out = emitOf(
		"const M: f32mat2 = f32mat2(1.0, 2.0, 3.0, 4.0);\nconst N: f32mat2 = f32mat2(M);\n"
		"fn main() {\n\tposition = f32vec4(N[1], 0.0, 1.0);\n}\n",
		EntryKind::eVertex
	);
	String log {};
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "OpConstantComposite")) << out.text;
}

TEST(IrSpirvEmitTest, MatrixFromMatrixAndBuiltinCallIndex) {
	const auto out = emitOf(
		"@input (mode = bind, id = \"u\", class = uniform) block u { f32mat4 m; };\n"
		"fn main() {\n\tf32mat3 a = f32mat3(u.m);\n\tf32vec4 c = inverse(u.m)[0];\n"
		"\tposition = f32vec4(a[0], 1.0) + c;\n}\n",
		EntryKind::eVertex
	);
	String log {};
	EXPECT_TRUE(out.result.diagnostics.empty());
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "MatrixInverse")) << out.text;
	EXPECT_TRUE(has(out.text, "OpCompositeExtract")) << out.text;
}

TEST(IrSpirvEmitTest, SpecConstantDefaultFoldsToLiteral) {
	const auto out = emitOf(
		"const PI: f32 = 3.0;\n"
		"@spec (id = \"delta\") const DELTA: f32 = -2.0 * PI / f32(4);\n"
		"fn main() {\n\tposition = f32vec4(DELTA);\n}\n",
		EntryKind::eVertex
	);
	String log {};
	EXPECT_TRUE(out.result.diagnostics.empty());
	EXPECT_TRUE(valid(out.result.words, log)) << log << "\n" << out.text;
	EXPECT_TRUE(has(out.text, "OpSpecConstant %float -1.5")) << out.text;
}

TEST(IrSpirvEmitTest, SpecConstantDefaultOverSpecIsNotConstant) {
	const auto prepared = prepare(
		"@spec (id = \"a\") const A: f32 = 1.0;\n"
		"@spec (id = \"b\") const B: f32 = A * 2.0;\n"
		"fn main() {\n\tposition = f32vec4(B);\n}\n",
		EntryKind::eVertex
	);
	ASSERT_TRUE(prepared.has_value());
	const auto result = spirv::emit(prepared->mod, prepared->layout, Tier::eDev);
	ASSERT_EQ(result.diagnostics.size(), 1uz);
	EXPECT_EQ(result.diagnostics[0].code()->key, "not-constant");
}

TEST(IrSpirvEmitTest, FloatOutOfIntegerRangeIsNotConstant) {
	for (const auto* const init : { "s32(3000000000.0)", "u32(-2.0)", "s32(Z / Z)" }) {
		const auto prepared = prepare(
			String { "const Z: f32 = 0.0;\nconst X: " } + (init[0] == 'u' ? "u32" : "s32") + " = " + init + ";\n"
			"fn main() {\n\tposition = f32vec4(f32(X));\n}\n",
			EntryKind::eVertex
		);
		EXPECT_TRUE(prepared.has_value()) << init;
		if (not prepared.has_value()) {
			continue;
		}
		const auto result = spirv::emit(prepared->mod, prepared->layout, Tier::eDev);
		ASSERT_EQ(result.diagnostics.size(), 1uz) << init;
		EXPECT_EQ(result.diagnostics[0].code()->key, "not-constant") << init;
		// the initializer on line 2, not the entry function on line 3
		EXPECT_EQ(result.diagnostics[0].primary().as<engine::diag::SourceSpan>().line, 2uL) << init;
	}
}

TEST(IrSpirvEmitTest, SpecFloatOutOfIntegerRangeIsDiagnosedAtInitializer) {
	const auto prepared = prepare(
		"const Z: f32 = 0.0;\n"
		"@spec (id = \"x\") const X: s32 = s32(3000000000.0);\n"
		"fn main() {\n\tposition = f32vec4(f32(X));\n}\n",
		EntryKind::eVertex
	);
	ASSERT_TRUE(prepared.has_value());
	const auto result = spirv::emit(prepared->mod, prepared->layout, Tier::eDev);
	ASSERT_EQ(result.diagnostics.size(), 1uz);
	EXPECT_EQ(result.diagnostics[0].code()->key, "not-constant");
	// the initializer on line 2, not the entry function on line 3
	EXPECT_EQ(result.diagnostics[0].primary().as<engine::diag::SourceSpan>().line, 2uL);
}

TEST(IrSpirvEmitTest, UserParamWithDoubleUnderscoreIsNamedInDevTier) {
	const auto src = String {
		"@input (mode = bind, id = \"o\", class = storage, access = read_write) block o { u32 v; };\n"
		"fn bump(a__b: u32) -> u32 { return a__b + 1u; }\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { o.v = bump(o.v); }\n"
	};
	EXPECT_TRUE(has(emitOf(src, EntryKind::eCompute).text, "a__b"));
	EXPECT_FALSE(has(emitOf(src, EntryKind::eCompute, Tier::eBuild).text, "OpName"));
}

TEST(IrSpirvEmitTest, MatrixStrideFollowsLayout) {
	const auto uniform = emitOf(
		"@input (mode = bind, id = \"u\", class = uniform) block u { f32mat2 m; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { f32mat2 x = u.m; }\n",
		EntryKind::eCompute
	);
	EXPECT_TRUE(has(uniform.text, "MatrixStride 16")) << uniform.text;
	EXPECT_TRUE(has(uniform.text, "ColMajor")) << uniform.text;

	const auto storage = emitOf(
		"@input (mode = bind, id = \"u\", class = storage) block u { f32mat2 m; };\n"
		"@compute (local_size = [1, 1, 1])\nfn main() { f32mat2 x = u.m; }\n",
		EntryKind::eCompute
	);
	EXPECT_TRUE(has(storage.text, "MatrixStride 8")) << storage.text;
}
