#include "pch.h"

#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Resolve.hpp>
#include <Engine.Accel.Lang/Ir/Uniformity.hpp>

using namespace hg;
using namespace hg::engine::accel::lang;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] Module parseOk(StringView source_, EntryKind entry_ = EntryKind::eFragment) {
			auto res = parseModule("M", source_, entry_, ModuleResolver { noIncludes });
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? std::move(res).value() : Module {};
		}

		/* uniformity of the first statement of kind `kind_` whose `name_`/operand text matches; see callers */
		[[nodiscard]] bool uniformOfCall(ref<const Module> module_, const Uniformity& u_, StringView callee_) {
			for (auto i = 0uz; i < module_.stmts.size(); ++i) {
				const auto& s = module_.stmts[i];
				if (s.kind == StmtKind::eExpr && s.lhs.index < module_.exprs.size() && module_.exprs[s.lhs.index].name == callee_) {
					return u_.uniformStmt[i];
				}
			}
			ADD_FAILURE() << "no call statement " << callee_;
			return false;
		}

		constexpr auto head =
			"@input (mode = bind, id = \"p\", class = push) block p { f32 x; };\n"
			"@input (mode = forward, id = \"in_x\") f32 in_x;\n"
			"fn a() {}\nfn b() {}\nfn c() {}\n";
	}

	TEST(UniformityTest, ModuleAndSpecConstantsAreUniform) {
		const auto m = parseOk(
			String { head } + "const K: f32 = 1.0;\n@spec (id = \"s\") const S: f32 = 2.0;\n"
			"fn main() {\n\tif (K > 0.0) { a(); }\n\tif (S > 0.0) { b(); }\n}\n"
		);
		const auto u = analyzeUniformity(m);
		EXPECT_TRUE(uniformOfCall(m, u, "a"));
		EXPECT_TRUE(uniformOfCall(m, u, "b"));
	}

	TEST(UniformityTest, SparseTexelIsLaneDependent) {
		const auto m = parseOk(
			String { head } +
			"@input (mode = bind, id = \"t2\", class = texture) texture2d t2;\n"
			"@input (mode = bind, id = \"smp\", class = sampler) sampler smp;\n"
			"fn main() {\n\tlet texel = f32vec4(0.0);\n\tlet r = sample_sparse(t2, smp, f32vec2(0.5, 0.5), texel);\n"
			"\tif (texel.x > 0.0) { a(); }\n}\n"
		);
		const auto u = analyzeUniformity(m);
		EXPECT_FALSE(uniformOfCall(m, u, "a"));
	}

	TEST(UniformityTest, PushConditionIsUniform) {
		const auto m = parseOk(String { head } + "fn main() {\n\tif (p.x > 0.0) { a(); }\n}\n");
		const auto u = analyzeUniformity(m);
		EXPECT_TRUE(uniformOfCall(m, u, "a"));
	}

	TEST(UniformityTest, ForwardInputConditionIsLaneDependent) {
		const auto m = parseOk(String { head } + "fn main() {\n\tif (in_x > 0.0) { a(); }\n}\n");
		const auto u = analyzeUniformity(m);
		EXPECT_FALSE(uniformOfCall(m, u, "a"));
	}

	TEST(UniformityTest, DivergentBreakTaintsRestOfLoop) {
		const auto m = parseOk(
			String { head } +
			"fn main() {\n\twhile (p.x > 0.0) {\n\t\tif (in_x > 0.0) { break; }\n\t\ta();\n\t}\n\tb();\n}\n"
		);
		const auto u = analyzeUniformity(m);
		EXPECT_FALSE(uniformOfCall(m, u, "a"));
		EXPECT_TRUE(uniformOfCall(m, u, "b"));
	}

	TEST(UniformityTest, ForDivergentContinueTaintsStepAndBody) {
		const auto m = parseOk(
			String { head } +
			"fn main() {\n\tfor (let i = 0.0; i < p.x; i += 2.0) {\n\t\tif (in_x > 0.0) { continue; }\n\t\ta();\n\t}\n\tb();\n}\n"
		);
		const auto u = analyzeUniformity(m);
		EXPECT_FALSE(uniformOfCall(m, u, "a"));
		EXPECT_TRUE(uniformOfCall(m, u, "b"));
		auto found = false;
		for (auto i = 0uz; i < m.stmts.size(); ++i) {
			const auto& s = m.stmts[i];
			if (s.kind == StmtKind::eAssign && s.name == "+=") {
				found = true;
				EXPECT_FALSE(u.uniformStmt[i]);
			}
		}
		EXPECT_TRUE(found);
	}

	TEST(UniformityTest, InnerLoopBreakDoesNotTaintOuterBody) {
		const auto m = parseOk(
			String { head } +
			"fn main() {\n\twhile (p.x > 0.0) {\n\t\twhile (p.x > 1.0) {\n\t\t\tif (in_x > 0.0) { break; }\n\t\t\ta();\n\t\t}\n\t\tb();\n\t}\n}\n"
		);
		const auto u = analyzeUniformity(m);
		EXPECT_FALSE(uniformOfCall(m, u, "a"));
		EXPECT_TRUE(uniformOfCall(m, u, "b"));
	}

	TEST(UniformityTest, ReturnInLoopTaintsAfterLoop) {
		const auto m = parseOk(
			String { head } +
			"fn main() {\n\twhile (p.x > 0.0) {\n\t\tif (in_x > 0.0) { return; }\n\t}\n\tb();\n}\n"
		);
		const auto u = analyzeUniformity(m);
		EXPECT_FALSE(uniformOfCall(m, u, "b"));
	}

	TEST(UniformityTest, DivergentReturnInLoopTaintsEarlierBody) {
		const auto m = parseOk(
			String { head } +
			"fn main() {\n\twhile (p.x > 0.0) {\n\t\ta();\n\t\tif (in_x > 0.0) { return; }\n\t}\n}\n"
		);
		const auto u = analyzeUniformity(m);
		EXPECT_FALSE(uniformOfCall(m, u, "a"));
	}

	TEST(UniformityTest, UniformIfWrappingDivergentBreak) {
		const auto m = parseOk(
			String { head } +
			"fn main() {\n\twhile (p.x > 0.0) {\n\t\tif (p.x > 2.0) {\n\t\t\tif (in_x > 0.0) { break; }\n\t\t}\n\t\ta();\n\t}\n\tb();\n}\n"
		);
		const auto u = analyzeUniformity(m);
		EXPECT_FALSE(uniformOfCall(m, u, "a"));
		EXPECT_TRUE(uniformOfCall(m, u, "b"));
	}

	TEST(UniformityTest, WriteThroughAliasTaintsRoot) {
		const auto m = parseOk(
			String { head } +
			"fn main() {\n\tlet x = 0.0;\n\tlet r = &mut x;\n\tif (in_x > 0.0) { r = 1.0; }\n\tif (x > 0.0) { a(); }\n}\n"
		);
		const auto u = analyzeUniformity(m);
		EXPECT_FALSE(uniformOfCall(m, u, "a"));
	}

	TEST(UniformityTest, BuiltinUnderDivergentLocalDeclIsDetected) {
		const auto m = parseOk(String { head } + "fn d() {\n\tif (in_x > 0.0) { let s = position; }\n}\nfn main() {\n\ta();\n}\n", EntryKind::eVertex);
		const auto u = analyzeUniformity(m, [](StringView n_) { return n_ == "position"; });
		auto count = 0uz;
		for (const auto flag : u.requiresUniform) {
			count += flag ? 1uz : 0uz;
		}
		EXPECT_EQ(count, 1uz);
	}

	TEST(UniformityTest, DivergentReturnTaintsRestOfFunction) {
		const auto m = parseOk(
			String { head } + "fn main() {\n\ta();\n\tif (in_x > 0.0) { return; }\n\tb();\n}\n"
		);
		const auto u = analyzeUniformity(m);
		EXPECT_TRUE(uniformOfCall(m, u, "a"));
		EXPECT_FALSE(uniformOfCall(m, u, "b"));
	}

	TEST(UniformityTest, LocalAssignedUnderDivergenceIsLaneDependent) {
		const auto m = parseOk(
			String { head } +
			"fn main() {\n\tlet v = 0.0;\n\tif (in_x > 0.0) { v = 1.0; }\n\tif (v > 0.0) { a(); }\n\tlet w = 0.0;\n\tif (w > 0.0) { b(); }\n}\n"
		);
		const auto u = analyzeUniformity(m);
		EXPECT_FALSE(uniformOfCall(m, u, "a"));
		EXPECT_TRUE(uniformOfCall(m, u, "b"));
	}

	TEST(UniformityTest, RequiresUniformPropagatesOverCalls) {
		const auto m = parseOk(String { head } + "fn d() { let s = position; }\nfn e() { d(); }\nfn main() {\n\ta();\n}\n", EntryKind::eVertex);
		const auto none = analyzeUniformity(m);
		for (const auto flag : none.requiresUniform) {
			EXPECT_FALSE(flag);
		}
		const auto u = analyzeUniformity(m, [](StringView n_) { return n_ == "position"; });
		auto count = 0uz;
		for (const auto flag : u.requiresUniform) {
			count += flag ? 1uz : 0uz;
		}
		EXPECT_EQ(count, 2uz);
	}
}
