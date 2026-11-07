#include "pch.h"

#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Typecheck.hpp>

using namespace hg;
using namespace hg::engine::accel::lang;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] String errorOf(StringView source_, EntryKind entry_) {
			auto res = parseModule("M", source_, entry_, ModuleResolver { noIncludes });
			return res.has_value() || res.error().empty() ? String {} : res.error().front().message;
		}

		[[nodiscard]] bool parses(StringView source_, EntryKind entry_ = EntryKind::eFragment) {
			auto res = parseModule("M", source_, entry_, ModuleResolver { noIncludes });
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value();
		}
	}

	TEST(BuiltinTest, ClampFamilyAcceptsVectorAndScalar) {
		EXPECT_TRUE(parses("fn main() {\n\tlet v = f32vec3(1.0);\n\tlet a = clamp(v, f32vec3(0.0), f32vec3(1.0));\n\tlet b = clamp(v, 0.0, 1.0);\n\tlet c = clamp(0.5, 0.0, 1.0);\n}\n"));
	}

	TEST(BuiltinTest, DotReturnsScalar) {
		auto res = parseModule("M", "fn main() {\n\tlet v = f32vec3(1.0);\n\tlet d = dot(v, v);\n}\n", EntryKind::eFragment, ModuleResolver { noIncludes });
		ASSERT_TRUE(res.has_value());
		for (const auto& stmt : res->stmts) {
			if (stmt.kind == StmtKind::eDeclareLocal && stmt.name == "d") {
				EXPECT_EQ(describeType(res->types, stmt.type), "f32");
			}
		}
	}

	TEST(BuiltinTest, DerivativeInVertexIsDiagnostic) {
		EXPECT_EQ(
			errorOf("fn main() {\n\tlet a = dFdx(1.0);\n}\n", EntryKind::eVertex),
			"'dFdx' is not available in vertex stages"
		);
	}

	TEST(BuiltinTest, WritingPositionInFragmentIsDiagnostic) {
		EXPECT_FALSE(errorOf("fn main() {\n\tposition = f32vec4(0.0);\n}\n", EntryKind::eFragment).empty());
		EXPECT_FALSE(errorOf("fn main() {\n\tfrag_coord = f32vec4(0.0);\n}\n", EntryKind::eFragment).empty());
	}

	TEST(BuiltinTest, Mat3FromMat4) {
		EXPECT_TRUE(parses("fn main() {\n\tlet m = f32mat4(1.0);\n\tlet n = f32mat3(m);\n\tlet c = f32mat3(f32vec3(1.0), f32vec3(2.0), f32vec3(3.0));\n}\n"));
	}

	TEST(BuiltinTest, MatrixScalarListConstructor) {
		EXPECT_TRUE(parses("fn main() {\n\tlet m = f32mat2(1.0, 0.0, 0.0, 1.0);\n}\n"));
		EXPECT_FALSE(errorOf("fn main() {\n\tlet m = f32mat2(1.0, 0.0, 0.0);\n}\n", EntryKind::eFragment).empty());
	}

	TEST(BuiltinTest, MatrixNonNumericScalarIsDiagnostic) {
		EXPECT_FALSE(errorOf("fn main() {\n\tlet m = f32mat3(true);\n}\n", EntryKind::eFragment).empty());
		EXPECT_FALSE(errorOf("fn main() {\n\tlet m = f32mat2(true, false, true, false);\n}\n", EntryKind::eFragment).empty());
	}

	TEST(BuiltinTest, LibraryWithoutMainSkipsStageCheck) {
		const auto source = "fn Lib() {\n\tposition = f32vec4(0.0);\n}\n";
		auto res = parseModule("M", source, EntryKind::eFragment, ModuleResolver { noIncludes });
		ASSERT_TRUE(res.has_value());
		EXPECT_FALSE(checkStageValidity(*res, EntryKind::eFragment).empty());
		EXPECT_TRUE(checkStageValidity(*res, EntryKind::eVertex).empty());
	}

	namespace {
		constexpr auto texturePrelude =
			"@input (mode = bind, id = \"t2\", class = texture) texture2d t2;\n"
			"@input (mode = bind, id = \"tc\", class = texture) texturecube tc;\n"
			"@input (mode = bind, id = \"smp\", class = sampler) sampler smp;\n"
			"@input (mode = forward, id = \"uv\") f32vec2 uv;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n";

		[[nodiscard]] Diagnostics warningsOf(String body_) {
			auto res = parseModule("M", String { texturePrelude } + body_, EntryKind::eFragment, ModuleResolver { noIncludes });
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? res->warnings : Diagnostics {};
		}
	}

	TEST(BuiltinTest, TextureSizeOnCubeIsS32Vec2) {
		auto res = parseModule(
			"M",
			String { texturePrelude } + "fn main() {\n\tlet n = texture_size(tc, smp, 0);\n\tcolor = f32vec4(0.0);\n}\n",
			EntryKind::eFragment,
			ModuleResolver { noIncludes }
		);
		ASSERT_TRUE(res.has_value());
		auto seen = false;
		for (const auto& stmt : res->stmts) {
			if (stmt.kind == StmtKind::eDeclareLocal && stmt.name == "n") {
				EXPECT_EQ(describeType(res->types, stmt.type), "s32vec2");
				seen = true;
			}
		}
		EXPECT_TRUE(seen);
	}

	TEST(BuiltinTest, SampleSparseNeedsMutRef) {
		EXPECT_TRUE(parses(String { texturePrelude } + "fn main() {\n\tlet texel = f32vec4(0.0);\n\tlet r = sample_sparse(t2, smp, uv, texel);\n\tcolor = texel;\n}\n"));
		EXPECT_FALSE(errorOf(String { texturePrelude } + "fn main() {\n\tconst texel = f32vec4(0.0);\n\tlet r = sample_sparse(t2, smp, uv, texel);\n\tcolor = texel;\n}\n", EntryKind::eFragment).empty());
		EXPECT_FALSE(errorOf(String { texturePrelude } + "fn main() {\n\tlet r = sample_sparse(t2, smp, uv, f32vec4(0.0));\n\tcolor = f32vec4(0.0);\n}\n", EntryKind::eFragment).empty());
	}

	TEST(BuiltinTest, ImplicitLodSampleUnderDivergenceWarns) {
		const auto warnings = warningsOf(
			"fn shadowReprojectFilter(p: f32vec2) -> f32vec4 {\n\treturn sample(t2, smp, p);\n}\n"
			"fn main() {\n\tcolor = f32vec4(0.0);\n\tif (uv.x > 0.5) {\n\t\tcolor = shadowReprojectFilter(uv);\n\t}\n}\n"
		);
		ASSERT_EQ(warnings.size(), 1uz);
		EXPECT_EQ(warnings.front().severity, Severity::eWarning);
		EXPECT_EQ(
			warnings.front().message,
			"'sample' uses implicit derivatives under lane-dependent control (via shadowReprojectFilter); results are undefined, use sample_lod"
		);

		const auto direct = warningsOf("fn main() {\n\tcolor = f32vec4(0.0);\n\tif (uv.x > 0.5) {\n\t\tcolor = sample(t2, smp, uv);\n\t}\n}\n");
		ASSERT_EQ(direct.size(), 1uz);
		EXPECT_EQ(direct.front().message, "'sample' uses implicit derivatives under lane-dependent control; results are undefined, use sample_lod");
	}

	TEST(BuiltinTest, SampleLodUnderDivergenceIsSilent) {
		EXPECT_TRUE(warningsOf("fn main() {\n\tcolor = f32vec4(0.0);\n\tif (uv.x > 0.5) {\n\t\tcolor = sample_lod(t2, smp, uv, 0.0);\n\t}\n}\n").empty());
	}

	TEST(BuiltinTest, SampleUnderUniformLoopIsSilent) {
		EXPECT_TRUE(
			warningsOf("fn main() {\n\tcolor = f32vec4(0.0);\n\tfor (let i = 0; i < 4; ++i) {\n\t\tcolor += sample(t2, smp, uv);\n\t}\n}\n").empty()
		);
	}

	TEST(BuiltinTest, UserFunctionShadowsBuiltin) {
		EXPECT_TRUE(parses("fn dot(a: f32, b: f32) -> f32 {\n\treturn a;\n}\nfn main() {\n\tlet d = dot(1.0, 2.0);\n}\n"));
		auto res = parseModule("M", "fn dot(a: f32, b: f32) -> f32 {\n\treturn a;\n}\nfn main() {\n\tlet d = dot(1.0, 2.0);\n}\n", EntryKind::eFragment, ModuleResolver { noIncludes });
		ASSERT_TRUE(res.has_value());
		for (const auto& e : res->exprs) {
			EXPECT_NE(e.kind, ExprKind::eBuiltin);
		}
	}
}
