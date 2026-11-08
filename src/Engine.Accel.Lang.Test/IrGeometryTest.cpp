#include "pch.h"

#include <Engine.Accel.Lang/Emitter/GlslEmitter.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Variant.hpp>

using namespace hg::engine::accel::lang::ir;
using namespace hg::engine::accel::lang;
using namespace hg;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] std::expected<Module, Diagnostics> parse(StringView source_, EntryKind entry_ = EntryKind::eGeometry) {
			return parseModule("M", source_, entry_, ModuleResolver { noIncludes });
		}

		[[nodiscard]] Module parseOk(StringView source_, EntryKind entry_ = EntryKind::eGeometry) {
			auto res = parse(source_, entry_);
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? std::move(res).value() : Module {};
		}

		[[nodiscard]] String errorOf(StringView source_, EntryKind entry_ = EntryKind::eGeometry) {
			auto res = parse(source_, entry_);
			return res.has_value() || res.error().empty() ? String {} : res.error().front().message;
		}

		[[nodiscard]] String emitGlsl(ref<Module> mod_) {
			Language target {};
			const auto layout = assignLayout(computeInterface(mod_), mod_.types, {});
			(void)GlslEmitter {}.emit(mod_, layout, target);
			return target.text.empty() ? String {} : String { target.text.front().data };
		}

		[[nodiscard]] String geometrySource(StringView attributes_) {
			return String { "@input (mode = forward, id = \"world-pos\") f32vec4[] p;\n" } + "@geometry (" + String { attributes_ } + ")\n" +
				"fn main() {\n\tposition = input_position[0];\n\temit_vertex();\n\tend_primitive();\n}\n";
		}
	}

	TEST(GeometryTest, ParsesPrimitiveAttributes) {
		const auto mod = parseOk(geometrySource("input = triangles, output = triangle_strip, max_vertices = 3, invocations = 2"));
		EXPECT_EQ(mod.attributes.geometryInput, Primitive::eTriangles);
		EXPECT_EQ(mod.attributes.geometryOutput, Primitive::eTriangleStrip);
		EXPECT_EQ(mod.attributes.maxVertices, 3u);
		EXPECT_EQ(mod.attributes.invocations, 2u);
		EXPECT_EQ(parseOk(geometrySource("input = points, output = points, max_vertices = 1")).attributes.invocations, 1u);

		// contradiction with the entry kind, missing required key
		EXPECT_FALSE(errorOf(geometrySource("input = triangles, output = triangle_strip, max_vertices = 3"), EntryKind::eFragment).empty());
		EXPECT_FALSE(errorOf(geometrySource("input = triangles, output = triangle_strip")).empty());
	}

	TEST(GeometryTest, EmitVertexOutsideGeometryIsDiagnostic) {
		EXPECT_FALSE(errorOf("fn main() {\n\temit_vertex();\n}\n", EntryKind::eFragment).empty());
		EXPECT_FALSE(errorOf("fn main() {\n\tend_primitive();\n}\n", EntryKind::eVertex).empty());
	}

	TEST(GeometryTest, ArrayedInputEmitsUnsizedArray) {
		auto mod = parseOk(geometrySource("input = triangles, output = triangle_strip, max_vertices = 3, invocations = 2"));
		ASSERT_FALSE(mod.bindings.empty());
		EXPECT_TRUE(mod.bindings.front().arrayed);
		const auto glsl = emitGlsl(mod);
		EXPECT_TRUE(glsl.contains("layout(triangles) in;")) << glsl;
		EXPECT_TRUE(glsl.contains("layout(triangle_strip, max_vertices = 3) out;")) << glsl;
		EXPECT_TRUE(glsl.contains("layout(invocations = 2) in;")) << glsl;
		EXPECT_TRUE(glsl.contains(" p[];")) << glsl;
		EXPECT_TRUE(glsl.contains("gl_in[0].gl_Position")) << glsl;
		EXPECT_TRUE(glsl.contains("EmitVertex();")) << glsl;
		EXPECT_TRUE(glsl.contains("EndPrimitive();")) << glsl;
	}

	TEST(GeometryTest, MaxVerticesChangesVariantKey) {
		const auto a = parseOk(geometrySource("input = triangles, output = triangle_strip, max_vertices = 3"));
		const auto b = parseOk(geometrySource("input = triangles, output = triangle_strip, max_vertices = 4"));
		const auto c = parseOk(geometrySource("input = triangles, output = line_strip, max_vertices = 3"));
		EXPECT_NE(contentHash(a), contentHash(b));
		EXPECT_NE(contentHash(a), contentHash(c));
	}
}
