#include "pch.h"

#include <Engine.Accel.Lang/Emitter/GlslEmitter.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
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

		[[nodiscard]] std::expected<Module, Diagnostics> parse(StringView source_, EntryKind entry_) {
			return parseModule("M", source_, entry_, ModuleResolver { noIncludes });
		}

		[[nodiscard]] Module parseOk(StringView source_, EntryKind entry_) {
			auto res = parse(source_, entry_);
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? std::move(res).value() : Module {};
		}

		[[nodiscard]] String errorOf(StringView source_, EntryKind entry_) {
			auto res = parse(source_, entry_);
			return res.has_value() || res.error().empty() ? String {} : res.error().front().message;
		}

		[[nodiscard]] String emitGlsl(ref<Module> module_) {
			Language target {};
			const auto layout = assignLayout(computeInterface(module_), module_.types, {});
			(void)GlslEmitter {}.emit(module_, layout, target);
			return target.text.empty() ? String {} : String { target.text.front().data };
		}

		[[nodiscard]] String ctrlSource(StringView write_) {
			return String { "@input (mode = forward, id = \"world-pos\") f32vec4[] p;\n" } +
				"@output (mode = forward, id = \"ctrl-pos\") f32vec4[] q;\n" +
				"@output (mode = forward, id = \"level\", patch = true) f32 lvl;\n" + "@tess_ctrl (output_vertices = 3)\n" +
				"fn main() {\n\t" + String { write_ } +
				"\n\toutput_position[invocation_id] = input_position[invocation_id];\n\tlvl = 2.0;\n"
				"\ttess_level_outer[0] = 2.0;\n\ttess_level_inner[0] = 2.0;\n}\n";
		}

		[[nodiscard]] String evalSource(StringView spacing_) {
			return String { "@input (mode = forward, id = \"ctrl-pos\") f32vec4[] q;\n" } +
				"@input (mode = forward, id = \"level\", patch = true) f32 lvl;\n" +
				"@tess_eval (domain = triangles, spacing = " + String { spacing_ } + ", winding = cw)\n" +
				"fn main() {\n\tposition = q[0] * tess_coord.x + q[1] * tess_coord.y + q[2] * tess_coord.z;\n}\n";
		}
	}

	TEST(TessellationTest, ParsesControlAndEvalAttributes) {
		const auto ctrl = parseOk(ctrlSource("q[invocation_id] = p[invocation_id];"), EntryKind::eTessCtrl);
		EXPECT_EQ(ctrl.attributes.outputVertices, 3u);
		const auto eval = parseOk(evalSource("fractional_odd"), EntryKind::eTessEval);
		EXPECT_EQ(eval.attributes.tessDomain, TessDomain::eTriangles);
		EXPECT_EQ(eval.attributes.tessSpacing, TessSpacing::eFractionalOdd);
		EXPECT_TRUE(eval.attributes.clockwise);

		EXPECT_FALSE(errorOf(ctrlSource("q[invocation_id] = p[invocation_id];"), EntryKind::eFragment).empty());
		EXPECT_FALSE(errorOf(evalSource("bogus"), EntryKind::eTessEval).empty());
	}

	TEST(TessellationTest, PerVertexWriteAtOtherIndexIsDiagnostic) {
		EXPECT_FALSE(errorOf(ctrlSource("q[0] = p[0];"), EntryKind::eTessCtrl).empty());
		EXPECT_FALSE(errorOf(
			String { "@output (mode = forward, id = \"o\") f32vec4[] q;\n@tess_ctrl (output_vertices = 3)\nfn main() {\n\t"
				"output_position[1] = input_position[0];\n}\n" },
			EntryKind::eTessCtrl
		).empty());
	}

	TEST(TessellationTest, PerVertexFieldAndNestedWritesAreChecked) {
		const auto ctrl = EntryKind::eTessCtrl;
		EXPECT_FALSE(errorOf(ctrlSource("output_position[1].x = 0.0;"), ctrl).empty());
		EXPECT_FALSE(errorOf(ctrlSource("q[0].x = 0.0;"), ctrl).empty());
		EXPECT_FALSE(errorOf(ctrlSource("q[0][1] = 0.0;"), ctrl).empty());
		EXPECT_TRUE(errorOf(ctrlSource("q[invocation_id].x = 0.0;"), ctrl).empty());
		EXPECT_TRUE(errorOf(ctrlSource("output_position[invocation_id].x = 0.0;"), ctrl).empty());
		EXPECT_TRUE(errorOf(ctrlSource("q[invocation_id][1] = 0.0;"), ctrl).empty());
	}

	TEST(TessellationTest, PatchIsOnlyValidOnTessForwardOutputsAndInputs) {
		const auto withAttr = [](String attr_, String src_) {
			const auto at = src_.find("fn main");
			return src_.substr(0, at) + attr_ + src_.substr(at);
		};
		const auto patchOut = String { "@output (mode = forward, id = \"o\", patch = true) f32 o;\nfn main() { o = 1.0; }\n" };
		const auto patchIn = String { "@input (mode = forward, id = \"i\", patch = true) f32 i;\nfn main() { }\n" };
		const auto badKind = String { "@input (mode = bind, id = \"u\", class = uniform, patch = true) block u { f32 v; };\nfn main() { }\n" };
		EXPECT_TRUE(errorOf(patchOut, EntryKind::eVertex).contains("patch"));
		EXPECT_TRUE(errorOf(patchIn, EntryKind::eTessCtrl).contains("patch"));
		EXPECT_TRUE(errorOf(withAttr("@tess_ctrl (output_vertices = 3)\n", patchIn), EntryKind::eTessCtrl).contains("patch"));
		EXPECT_TRUE(errorOf(withAttr("@tess_eval (domain = triangles, spacing = equal, winding = cw)\n", patchOut), EntryKind::eTessEval).contains("patch"));
		EXPECT_TRUE(errorOf(withAttr("@tess_ctrl (output_vertices = 3)\n", badKind), EntryKind::eTessCtrl).contains("patch"));
		EXPECT_FALSE(errorOf(withAttr("@tess_ctrl (output_vertices = 3)\n", patchOut), EntryKind::eTessCtrl).contains("patch"));
	}

	TEST(TessellationTest, PatchOutputEmitsPatchQualifier) {
		auto ctrl = parseOk(ctrlSource("q[invocation_id] = p[invocation_id];"), EntryKind::eTessCtrl);
		const auto glsl = emitGlsl(ctrl);
		EXPECT_TRUE(glsl.contains("layout(vertices = 3) out;")) << glsl;
		EXPECT_TRUE(glsl.contains("patch out ")) << glsl;
		EXPECT_TRUE(glsl.contains(" q[];")) << glsl;
		EXPECT_TRUE(glsl.contains("gl_out[gl_InvocationID].gl_Position")) << glsl;
		EXPECT_TRUE(glsl.contains("gl_TessLevelOuter[0]")) << glsl;

		auto eval = parseOk(evalSource("fractional_even"), EntryKind::eTessEval);
		const auto evalGlsl = emitGlsl(eval);
		EXPECT_TRUE(evalGlsl.contains("layout(triangles, fractional_even_spacing, cw) in;")) << evalGlsl;
		EXPECT_TRUE(evalGlsl.contains("patch in ")) << evalGlsl;
		EXPECT_TRUE(evalGlsl.contains("gl_TessCoord")) << evalGlsl;
	}
}
