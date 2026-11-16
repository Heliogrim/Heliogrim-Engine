#include "pch.h"

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
		EXPECT_TRUE(parsed.has_value());
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
	EXPECT_FALSE(has(out.text, "Unknown")) << out.text;
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
