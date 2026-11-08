#include "pch.h"

#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Place.hpp>

using namespace hg::engine::accel::lang::ir;
using namespace hg::engine::accel::lang;
using namespace hg;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		constexpr auto prelude =
			"@input (mode = bind, id = \"buf\", class = storage, access = read_write) block buf { u32 a; u32 b; u32[] data; };\n"
			"struct Inner { u32[16] d; }\n"
			"struct Outer { Inner[4] l; }\n"
			"workgroup a: u32[16];\n"
			"workgroup s: Outer;\n"
			"workgroup t: u32[64];\n";

		/* the place of an expression chain (names are not resolved; root scope stays unbound) */
		void walk(ref<const Module> module_, ExprRef ref_, ref<PlacePath> out_) {
			const auto& e = module_.exprs[ref_.index];
			if (e.kind == ExprKind::eNameRef || e.kind == ExprKind::eBuiltin) {
				out_.root = e.name;
				return;
			}
			walk(module_, e.operands[0], out_);
			if (e.kind == ExprKind::eField) {
				out_.steps.emplace_back(PlaceStep { .kind = PlaceStep::Kind::eField, .field = e.name });
			} else if (e.kind == ExprKind::eIndex) {
				out_.steps.emplace_back(PlaceStep { .kind = PlaceStep::Kind::eIndex, .index = e.operands[1] });
			} else {
				out_.steps.emplace_back(PlaceStep { .kind = PlaceStep::Kind::eRange, .lo = e.operands[1], .hi = e.operands[2] });
			}
		}

		/* `decls_` are statements declaring locals; the first two `let p = &mut <place>;` borrows are compared */
		[[nodiscard]] bool disjointOf(StringView locals_, StringView first_, StringView second_, bool boundsImmutable_ = true) {
			const auto source = String { prelude } + "fn main() {\n" + String { locals_ } + "\n{ let p = &mut " + String { first_ } + "; }\n{ let q = &mut " +
				String { second_ } + "; }\n}\n";
			auto res = parseModule("M", source, EntryKind::eCompute, ModuleResolver { noIncludes });
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? String {} : res.error().front().message);
			if (not res.has_value()) {
				return false;
			}
			const auto& module = *res;
			Vector<PlacePath> paths {};
			for (const auto& stmt : module.stmts) {
				if (stmt.kind == StmtKind::eDeclareRef) {
					paths.emplace_back();
					walk(module, stmt.lhs, paths.back());
				}
			}
			EXPECT_EQ(paths.size(), 2uz);
			return paths.size() == 2uz && disjoint(
				module,
				paths[0],
				paths[1],
				[&](ref<const Expr> e_) { return boundsImmutable_ && (e_.name == "n" || e_.name == "m"); }
			);
		}
	}

	TEST(PlaceTest, DifferentFieldsDisjoint) {
		EXPECT_TRUE(disjointOf("", "buf.a", "buf.b"));
		EXPECT_FALSE(disjointOf("", "buf.a", "buf.a"));
	}

	TEST(PlaceTest, ConstantIndicesDisjoint) {
		EXPECT_TRUE(disjointOf("", "a[0u]", "a[1u]"));
		EXPECT_FALSE(disjointOf("", "a[1u]", "a[1u]"));
	}

	TEST(PlaceTest, RuntimeIndicesOverlap) {
		EXPECT_FALSE(disjointOf("let i: u32 = 0u; let j: u32 = 1u;", "a[i]", "a[j]"));
	}

	TEST(PlaceTest, SharedImmutableBoundDisjoint) {
		EXPECT_TRUE(disjointOf("const n: u32 = 4u; const m: u32 = 8u;", "t[0u .. n]", "t[n .. m]"));
	}

	TEST(PlaceTest, SharedMutableBoundOverlaps) {
		EXPECT_FALSE(disjointOf("let n: u32 = 4u; let m: u32 = 8u;", "t[0u .. n]", "t[n .. m]", false));
	}

	TEST(PlaceTest, ConstantRangesDisjoint) {
		EXPECT_TRUE(disjointOf("", "t[0u .. 4u]", "t[4u .. 8u]"));
		EXPECT_FALSE(disjointOf("", "t[0u .. 5u]", "t[4u .. 8u]"));
	}

	TEST(PlaceTest, IndexOutsideConstantRange) {
		EXPECT_TRUE(disjointOf("", "t[9u]", "t[0u .. 8u]"));
		EXPECT_FALSE(disjointOf("", "t[3u]", "t[0u .. 8u]"));
	}

	TEST(PlaceTest, RuntimePrefixOverlaps) {
		EXPECT_FALSE(
			disjointOf(
				"const n: u32 = 4u; const m: u32 = 8u; let i: u32 = 0u; let j: u32 = 1u;",
				"s.l[i].d[0u .. n]",
				"s.l[j].d[n .. m]"
			)
		);
	}

	TEST(PlaceTest, PrefixOverlapsWhole) {
		EXPECT_FALSE(disjointOf("", "buf.data[0u .. 4u]", "buf.data[0u .. 4u]"));
		EXPECT_FALSE(disjointOf("", "s", "s.l[0u]"));
	}

	TEST(PlaceTest, SwizzlesShareComponents) {
		EXPECT_TRUE(disjointOf("let v = f32vec4(1.0);", "v.x", "v.y"));
		EXPECT_FALSE(disjointOf("let v = f32vec4(1.0);", "v.xy", "v.y"));
	}
}
