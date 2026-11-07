#include "pch.h"

#include <Engine.Accel.Lang/Emitter/GlslEmitter.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
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

		[[nodiscard]] std::expected<Module, Diagnostics> parse(StringView source_) {
			return parseModule("M", source_, EntryKind::eFragment, ModuleResolver { noIncludes });
		}

		[[nodiscard]] Module parseOk(StringView source_) {
			auto res = parse(source_);
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? std::move(res).value() : Module {};
		}

		[[nodiscard]] String firstMessage(StringView source_) {
			auto res = parse(source_);
			EXPECT_FALSE(res.has_value());
			return res.has_value() || res.error().empty() ? String {} : res.error().front().message;
		}

		[[nodiscard]] String typeNameOfLocal(ref<const Module> module_, StringView name_) {
			for (const auto& stmt : module_.stmts) {
				if (stmt.kind == StmtKind::eDeclareLocal && stmt.name == name_) {
					return describeType(module_.types, stmt.type);
				}
			}
			ADD_FAILURE() << "no local " << name_;
			return {};
		}
	}

	TEST(TypecheckTest, InfersLetFromInitializer) {
		const auto module = parseOk("fn main() {\n\tlet v = f32vec4(1.0);\n\tlet s = v.xy;\n}\n");
		EXPECT_EQ(typeNameOfLocal(module, "v"), "f32vec4");
		EXPECT_EQ(typeNameOfLocal(module, "s"), "f32vec2");
	}

	TEST(TypecheckTest, ResolvesOverloadByArgumentType) {
		const auto module = parseOk(
			"fn saturate(x: f32) -> f32 { return x; }\n"
			"fn saturate(x: f32vec3) -> f32vec3 { return x; }\n"
			"fn main() {\n\tlet v = f32vec3(1.0);\n\tlet r = saturate(v);\n}\n"
		);
		auto second = ~0u;
		for (auto i = 0uz; i < module.functions.size(); ++i) {
			if (module.functions[i].name == "saturate" && signatureOf(module, module.functions[i]) == "saturate(f32vec3)") {
				second = static_cast<u32>(i);
			}
		}
		ASSERT_NE(second, ~0u);
		auto found = false;
		for (const auto& expr : module.exprs) {
			if (expr.kind == ExprKind::eCall) {
				found = true;
				EXPECT_EQ(expr.function.index, second);
			}
		}
		EXPECT_TRUE(found);
		EXPECT_EQ(typeNameOfLocal(module, "r"), "f32vec3");
	}

	TEST(TypecheckTest, NoOverloadListsCandidates) {
		const auto message = firstMessage(
			"fn saturate(x: f32) -> f32 { return x; }\n"
			"fn saturate(x: f32vec3) -> f32vec3 { return x; }\n"
			"fn main() {\n\tlet i: s32 = 1;\n\tlet r = saturate(i);\n}\n"
		);
		EXPECT_TRUE(message.contains("no overload of 'saturate' takes (s32); candidates: saturate(f32), saturate(f32vec3)")) << message;
	}

	TEST(TypecheckTest, LiteralAdaptsToFloatContext) {
		auto module = parseOk("fn F(x: f32) -> f32 {\n\tif (x > 0) {\n\t\treturn x;\n\t}\n\treturn 0.5;\n}\nfn main() {\n\tfrag_depth = F(1.0);\n}\n");
		Language target {};
		const auto layout = assignLayout(computeInterface(module), module.types, {});
		const auto res = GlslEmitter {}.emit(module, layout, target);
		ASSERT_TRUE(res.diagnostics.empty()) << res.diagnostics.front().message;
		ASSERT_FALSE(target.text.empty());
		EXPECT_TRUE(target.text.front().data.contains("x > 0.0")) << target.text.front().data;
	}

	TEST(TypecheckTest, MixedIntFloatIsDiagnostic) {
		const auto message = firstMessage("fn F(a: s32, b: f32) -> f32 {\n\treturn a * b;\n}\n");
		EXPECT_TRUE(message.contains("operands of '*' have types s32 and f32")) << message;
	}

	TEST(TypecheckTest, MatrixVectorProductType) {
		const auto module = parseOk("fn F(m: f32mat4, v: f32vec4) -> f32vec4 {\n\tlet r = m * v;\n\treturn r;\n}\n");
		EXPECT_EQ(typeNameOfLocal(module, "r"), "f32vec4");
	}

	TEST(TypecheckTest, UnknownSwizzleComponent) {
		const auto message = firstMessage("fn F(v: f32vec2) -> f32 {\n\treturn v.xyz;\n}\n");
		EXPECT_TRUE(message.contains("unknown field 'xyz' of f32vec2")) << message;
	}
}
