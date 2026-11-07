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

	TEST(BuiltinTest, UserFunctionShadowsBuiltin) {
		EXPECT_TRUE(parses("fn dot(a: f32, b: f32) -> f32 {\n\treturn a;\n}\nfn main() {\n\tlet d = dot(1.0, 2.0);\n}\n"));
		auto res = parseModule("M", "fn dot(a: f32, b: f32) -> f32 {\n\treturn a;\n}\nfn main() {\n\tlet d = dot(1.0, 2.0);\n}\n", EntryKind::eFragment, ModuleResolver { noIncludes });
		ASSERT_TRUE(res.has_value());
		for (const auto& e : res->exprs) {
			EXPECT_NE(e.kind, ExprKind::eBuiltin);
		}
	}
}
