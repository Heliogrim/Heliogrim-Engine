#include <algorithm>
#include "pch.h"

#include <Engine.Accel.Lang/Emitter/GlslEmitter.hpp>
#include <Engine.Accel.Lang/Ir/Analysis.hpp>
#include <Engine.Accel.Lang/Ir/Compact.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Resolve.hpp>
#include <Engine.Accel.Lang/Ir/Slice.hpp>
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

		[[nodiscard]] String emitGlsl(ref<Module> module_) {
			Language target {};
			const auto layout = assignLayout(computeInterface(module_), module_.types, {});
			(void)GlslEmitter {}.emit(module_, layout, target);
			return target.text.empty() ? String {} : String { target.text.front().data };
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

	TEST(TypecheckTest, DiagnosticsCarryTheNodeLocation) {
		auto res = parse("fn F(v: f32vec2) -> f32 {\n\treturn v.xyz;\n}\n");
		ASSERT_FALSE(res.has_value());
		ASSERT_FALSE(res.error().empty());
		EXPECT_EQ(res.error().front().line, 2u);
		EXPECT_GT(res.error().front().column, 0u);
	}

	TEST(TypecheckTest, ParserReportsUnknownNameBeforeTypecheck) {
		EXPECT_TRUE(firstMessage("fn F() -> f32 {\n\treturn missing;\n}\n").contains("missing"));
	}

	namespace {
		constexpr auto sliceSource =
			"@output (mode = forward, id = \"depth\") implicit f32 depth;\n"
			"fn f(x: f32) -> f32 { return x; }\n"
			"fn f(x: f32vec3) -> f32vec3 { return x; }\n"
			"fn main() {\n\tdepth = f(f32vec3(1.0)).x;\n}\n";


		constexpr auto overloads =
			"fn f(x: f32) -> f32 { return x; }\n"
			"fn f(x: f32vec3) -> f32vec3 { return x; }\n"
			"fn main() {\n\tfrag_depth = f(f32vec3(1.0)).x;\n}\n";

		[[nodiscard]] Vector<String> signaturesOf(ref<const Module> module_, ref<const Vector<u32>> indices_) {
			Vector<String> out {};
			for (const auto i : indices_) {
				out.emplace_back(signatureOf(module_, module_.functions[i]));
			}
			return out;
		}

		[[nodiscard]] u32 indexOf(ref<const Module> module_, StringView signature_) {
			for (auto i = 0uz; i < module_.functions.size(); ++i) {
				if (signatureOf(module_, module_.functions[i]) == signature_) {
					return static_cast<u32>(i);
				}
			}
			ADD_FAILURE() << "no function " << signature_;
			return ~0u;
		}
	}

	TEST(TypecheckTest, ReachabilityFollowsTheChosenOverload) {
		const auto module = parseOk(overloads);
		const std::array<u32, 1> roots { indexOf(module, "main()") };
		const auto reached = reachableFunctions(module, roots);
		auto names = signaturesOf(module, reached);
		std::ranges::sort(names);
		EXPECT_EQ(names, (Vector<String> { "f(f32vec3)", "main()" }));
	}

	TEST(TypecheckTest, CompactKeepsTheCalledOverloadAndNeverSubstitutes) {
		auto module = parseOk(overloads);
		const auto main = indexOf(module, "main()");
		const auto vec = indexOf(module, "f(f32vec3)");
		const auto scalar = indexOf(module, "f(f32)");

		CompactPlan plan {};
		plan.stmtAlias.resize(module.stmts.size());
		for (auto i = 0uz; i < plan.stmtAlias.size(); ++i) {
			plan.stmtAlias[i] = static_cast<u32>(i);
		}
		plan.functions = vec < main ? Vector<u32> { vec, main } : Vector<u32> { main, vec };
		const auto kept = compact(module, plan);
		EXPECT_TRUE(kept.unresolvedCalls.empty());
		auto names = Vector<String> {};
		for (const auto& fn : kept.module.functions) {
			names.emplace_back(signatureOf(kept.module, fn));
		}
		std::ranges::sort(names);
		EXPECT_EQ(names, (Vector<String> { "f(f32vec3)", "main()" }));

		// the called overload is dropped: the call stays unresolved by signature, it must not bind the other overload
		plan.functions = scalar < main ? Vector<u32> { scalar, main } : Vector<u32> { main, scalar };
		const auto dropped = compact(module, plan);
		EXPECT_EQ(dropped.unresolvedCalls, (Vector<String> { "f(f32vec3)" }));
	}

	TEST(TypecheckTest, SliceKeepsTheCalledOverloadOnly) {
		const auto module = parseOk(sliceSource);
		const auto resolved = resolveToggles(module, ToggleSet {});
		ASSERT_TRUE(resolved.has_value());
		const std::array<String, 1> outputs { String { "depth" } };
		const auto sliced = slice(*resolved, std::span<const String> { outputs });
		Vector<String> names {};
		for (const auto& fn : sliced.functions) {
			names.emplace_back(signatureOf(sliced, fn));
		}
		std::ranges::sort(names);
		EXPECT_EQ(names, (Vector<String> { "f(f32vec3)", "main()" }));
	}

	namespace {
		[[nodiscard]] TypeRef addType(ref<Module> m_, TypeDesc d_) {
			m_.types.emplace_back(std::move(d_));
			return TypeRef { static_cast<u32>(m_.types.size() - 1uz) };
		}

		[[nodiscard]] ExprRef addExpr(ref<Module> m_, Expr e_) {
			m_.exprs.emplace_back(std::move(e_));
			return ExprRef { static_cast<u32>(m_.exprs.size() - 1uz) };
		}

		[[nodiscard]] StmtRef addStmt(ref<Module> m_, Stmt s_) {
			m_.stmts.emplace_back(std::move(s_));
			return StmtRef { static_cast<u32>(m_.stmts.size() - 1uz) };
		}

		[[nodiscard]] FunctionDecl bodied(ref<Module> m_, String name_, Vector<ParamDecl> params_, TypeRef ret_, StmtKind kind_, ExprRef value_) {
			const auto inner = addStmt(m_, Stmt { .kind = kind_, .lhs = value_ });
			const auto root = addStmt(m_, Stmt { .kind = StmtKind::eBlock, .children = { inner } });
			return FunctionDecl { .name = std::move(name_), .params = std::move(params_), .ret = ret_, .body = BodyRef { root } };
		}
	}

	TEST(TypecheckTest, UnresolvedNameRefIsReported) {
		Module module {};
		const auto name = addExpr(module, Expr { .kind = ExprKind::eNameRef, .name = "missing" });
		module.functions.emplace_back(bodied(module, "main", {}, {}, StmtKind::eExpr, name));

		const auto diagnostics = typecheck(module);
		ASSERT_FALSE(diagnostics.empty());
		EXPECT_EQ(diagnostics.front().message, "unknown name 'missing'");
	}

	TEST(TypecheckTest, IntegerMatrixIsNotNumeric) {
		Module module {};
		const auto mat = addType(module, TypeDesc { .kind = TypeKind::eMatrix, .scalar = ScalarKind::eInt, .n = 2uL, .m = 2uL });
		const auto x = addExpr(module, Expr { .kind = ExprKind::eNameRef, .name = "x", .local = true });
		const auto neg = addExpr(module, Expr { .kind = ExprKind::eUnary, .name = "-", .operands = { x } });
		module.functions.emplace_back(bodied(module, "f", { ParamDecl { .name = "x", .type = mat } }, mat, StmtKind::eReturn, neg));

		EXPECT_FALSE(typecheck(module).empty());
	}

	TEST(TypecheckTest, SameNamedStructsWithDifferentFieldsAreDistinct) {
		Module module {};
		const auto f32t = addType(module, TypeDesc { .kind = TypeKind::eScalar, .scalar = ScalarKind::eFloat });
		const auto i32t = addType(module, TypeDesc { .kind = TypeKind::eScalar, .scalar = ScalarKind::eInt });
		const auto s1 = addType(module, TypeDesc { .kind = TypeKind::eStruct, .name = "S", .fields = { FieldDesc { "a", f32t } } });
		const auto s2 = addType(module, TypeDesc { .kind = TypeKind::eStruct, .name = "S", .fields = { FieldDesc { "a", i32t } } });

		const auto noop = addStmt(module, Stmt { .kind = StmtKind::eBlock });
		module.functions.emplace_back(FunctionDecl { .name = "F", .params = { ParamDecl { .name = "x", .type = s1 } }, .body = BodyRef { noop } });
		module.functions.emplace_back(FunctionDecl { .name = "F", .params = { ParamDecl { .name = "x", .type = s2 } }, .body = BodyRef { noop } });

		const auto y = addExpr(module, Expr { .kind = ExprKind::eNameRef, .name = "y", .local = true });
		const auto call = addExpr(module, Expr { .kind = ExprKind::eCall, .name = "F", .operands = { y } });
		module.functions.emplace_back(bodied(module, "g", { ParamDecl { .name = "y", .type = s2 } }, {}, StmtKind::eExpr, call));

		EXPECT_TRUE(typecheck(module).empty());
		EXPECT_EQ(module.exprs[call.index].function.index, 1uL);
	}

	TEST(MutabilityTest, LetIsMutable) {
		parseOk("fn main() {\n\tlet v = 1.0;\n\tv = 2.0;\n}\n");
	}

	TEST(MutabilityTest, ConstIsImmutable) {
		EXPECT_TRUE(firstMessage("fn main() {\n\tconst k = 1.0;\n\tk = 2.0;\n}\n").contains("cannot assign to constant 'k'"));
	}

	TEST(MutabilityTest, ConstNeedsInitializer) {
		EXPECT_TRUE(firstMessage("fn main() {\n\tconst k: f32;\n}\n").contains("a 'const' local needs an initializer"));
	}

	TEST(MutabilityTest, ConstWithRuntimeInitializerIsValid) {
		const auto module = parseOk(
			"@input (mode = forward, id = \"v\") f32vec3 v;\n"
			"fn normalize(x: f32vec3) -> f32vec3 { return x; }\n"
			"fn main() {\n\tconst n = normalize(v);\n}\n"
		);
		auto module2 = module;
		const auto text = emitGlsl(module2);
		EXPECT_TRUE(text.contains("const vec3 n = normalize(v);")) << text;
	}

	TEST(MutabilityTest, TypedLocalIsMutable) {
		parseOk("fn main() {\n\tf32 x = 1.0;\n\tx = 2.0;\n}\n");
	}

	TEST(MutabilityTest, ConstMutAliasIsDiagnostic) {
		EXPECT_TRUE(
			firstMessage("fn main() {\n\tlet a = 1.0;\n\tconst l = &mut a;\n}\n").contains("cannot take '&mut' into a 'const' alias")
		);
	}

	TEST(MutabilityTest, ValueParamIsImmutable) {
		EXPECT_FALSE(parse("fn f(x: f32) { x = 1.0; }\nfn main() {}\n").has_value());
	}

	TEST(MutabilityTest, RefParamIsImmutable) {
		EXPECT_TRUE(firstMessage("fn f(x: &f32) { x = 1.0; }\nfn main() {}\n").contains("cannot assign through immutable reference 'x'"));
	}

	TEST(MutabilityTest, RefMutParamIsAssignable) {
		parseOk("fn f(x: &mut f32) { x = 1.0; }\nfn main() {}\n");
	}

	TEST(MutabilityTest, InputBindingIsReadOnly) {
		const auto head = String { "@input (mode = bind, id = \"camera\", class = storage) block camera { f32 x; };\n" };
		EXPECT_FALSE(parse(head + "fn main() {\n\tcamera.x = 1.0;\n}\n").has_value());
		EXPECT_TRUE(
			firstMessage(head + "fn main() {\n\tlet r = &mut camera.x;\n}\n").contains("cannot borrow read-only binding 'camera' as mutable")
		);
	}

	TEST(MutabilityTest, StorageReadWriteIsAssignable) {
		parseOk("@input (mode = bind, id = \"camera\", class = storage, access = read_write) block camera { f32 x; };\nfn main() {\n\tcamera.x = 1.0;\n}\n");
	}

	TEST(MutabilityTest, AliasingMutArgumentsIsDiagnostic) {
		EXPECT_TRUE(
			firstMessage("fn F(x: &mut f32, y: &f32) {}\nfn main() {\n\tlet a = 1.0;\n\tF(a, a);\n}\n")
			.contains("'&mut' argument 'a' aliases another argument of call to 'F'")
		);
	}
}
