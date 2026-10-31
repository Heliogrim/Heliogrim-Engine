#include "pch.h"

#include <Engine.Accel.Lang/Ir/Analysis.hpp>
#include <Engine.Accel.Lang/Ir/Module.hpp>

using namespace hg;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	namespace {
		[[nodiscard]] ExprRef addExpr(Module& module_, Expr expr_) {
			module_.exprs.emplace_back(std::move(expr_));
			return ExprRef { static_cast<u32>(module_.exprs.size() - 1uL) };
		}

		[[nodiscard]] ExprRef name(Module& module_, String name_) {
			return addExpr(module_, Expr { .kind = ExprKind::eNameRef, .name = std::move(name_) });
		}

		[[nodiscard]] StmtRef addStmt(Module& module_, Stmt stmt_) {
			module_.stmts.emplace_back(std::move(stmt_));
			return StmtRef { static_cast<u32>(module_.stmts.size() - 1uL) };
		}

		[[nodiscard]] StmtRef exprStmt(Module& module_, ExprRef value_) {
			return addStmt(module_, Stmt { .kind = StmtKind::eExpr, .lhs = value_ });
		}

		/* Appends a function whose body is a block of the given statements */
		u32 addFunction(Module& module_, String name_, Vector<StmtRef> stmts_) {
			const auto root = addStmt(module_, Stmt { .kind = StmtKind::eBlock, .children = std::move(stmts_) });
			module_.functions.emplace_back(FunctionDecl { .name = std::move(name_), .body = BodyRef { root } });
			return static_cast<u32>(module_.functions.size() - 1uL);
		}

		[[nodiscard]] StmtRef callStmt(Module& module_, String callee_) {
			return exprStmt(module_, addExpr(module_, Expr { .kind = ExprKind::eCall, .name = std::move(callee_) }));
		}

		/* main -> a -> b, c unused; b reads `camera`, writes `depth` */
		[[nodiscard]] Module makeModule() {
			Module m {};
			m.bindings.emplace_back(BindingDecl { .id = "camera", .localName = "camera" });
			m.bindings.emplace_back(BindingDecl { .id = "depth", .localName = "depth" });

			const auto lhs = name(m, "depth");
			const auto rhs = name(m, "camera");
			const auto write = addStmt(m, Stmt { .kind = StmtKind::eAssign, .lhs = lhs, .rhs = rhs });

			addFunction(m, "main", { callStmt(m, "a") });
			addFunction(m, "a", { callStmt(m, "b") });
			addFunction(m, "b", { write });
			addFunction(m, "c", {});
			return m;
		}
	}

	TEST(IrAnalysisTest, ReachableFollowsCalls) {
		const auto m = makeModule();
		const u32 roots[] = { 0uL };
		EXPECT_EQ(reachableFunctions(m, roots), (Vector<u32> { 0uL, 1uL, 2uL }));
	}

	TEST(IrAnalysisTest, UsageListsReadsWritesAndCalls) {
		const auto m = makeModule();

		const auto b = usageOf(m, 2uL);
		EXPECT_EQ(b.readBindings, (Vector<u32> { 0uL }));
		EXPECT_EQ(b.writtenBindings, (Vector<u32> { 1uL }));
		EXPECT_TRUE(b.calledFunctions.empty());

		EXPECT_EQ(usageOf(m, 0uL).calledFunctions, (Vector<u32> { 1uL }));
	}

	TEST(IrAnalysisTest, RecursionTerminates) {
		Module m {};
		addFunction(m, "a", { callStmt(m, "a") });

		const u32 roots[] = { 0uL };
		EXPECT_EQ(reachableFunctions(m, roots), (Vector<u32> { 0uL }));
		EXPECT_EQ(usageOf(m, 0uL).calledFunctions, (Vector<u32> { 0uL }));
	}

	TEST(IrAnalysisTest, LocalShadowingBindingIsNotBindingUse) {
		Module m {};
		m.bindings.emplace_back(BindingDecl { .id = "depth", .localName = "depth" });

		const auto lhs = addExpr(m, Expr { .kind = ExprKind::eNameRef, .name = "depth", .local = true });
		const auto rhs = addExpr(m, Expr { .kind = ExprKind::eNameRef, .name = "depth", .local = true });
		const auto write = addStmt(m, Stmt { .kind = StmtKind::eAssign, .lhs = lhs, .rhs = rhs });
		addFunction(m, "f", { write });

		const auto usage = usageOf(m, 0uL);
		EXPECT_TRUE(usage.readBindings.empty());
		EXPECT_TRUE(usage.writtenBindings.empty());
	}

	TEST(IrAnalysisTest, ResolvedIndexWinsOverName) {
		Module m {};
		m.bindings.emplace_back(BindingDecl { .id = "a", .localName = "a" });
		m.bindings.emplace_back(BindingDecl { .id = "b", .localName = "b" });

		addFunction(
			m,
			"f",
			{ exprStmt(m, addExpr(m, Expr { .kind = ExprKind::eNameRef, .name = "a", .binding = BindingRef { 1uL } })) }
		);

		EXPECT_EQ(usageOf(m, 0uL).readBindings, (Vector<u32> { 1uL }));
	}

	TEST(IrAnalysisTest, WhenCountsBothArms) {
		Module m {};
		m.bindings.emplace_back(BindingDecl { .id = "x", .localName = "x" });
		m.bindings.emplace_back(BindingDecl { .id = "y", .localName = "y" });

		const auto thenArm = exprStmt(m, name(m, "x"));
		const auto elseArm = exprStmt(m, name(m, "y"));
		const auto guarded = addStmt(
			m,
			Stmt {
				.kind = StmtKind::eWhen,
				.children = { thenArm, elseArm },
				.when = Condition::equals("T", "A")
			}
		);
		addFunction(m, "f", { guarded });

		EXPECT_EQ(usageOf(m, 0uL).readBindings, (Vector<u32> { 0uL, 1uL }));
	}
}
