#include "pch.h"

/**/

#include <Engine.Accel.Lang/Ir/Condition.hpp>
#include <Engine.Accel.Lang/Ir/Module.hpp>
#include <Engine.Accel.Lang/Ir/Place.hpp>

import std;

using namespace hg::engine::accel::lang::ir;
using namespace hg;

namespace AccelLang {
	namespace {
		[[nodiscard]] Toggle toggle(String name_, Vector<String> values_) {
			auto def = values_.front();
			return Toggle { std::move(name_), std::move(values_), std::move(def) };
		}
	}

	TEST(ConditionTest, EvaluatesCombinators) {
		// Shading==Forward && !(Shadows==On)
		const auto cond = Condition::all(
			{ Condition::equals("Shading", "Forward"), Condition::negate(Condition::equals("Shadows", "On")) }
		);

		EXPECT_TRUE(evaluate(cond, { { "Shading", "Forward" }, { "Shadows", "Off" } }));
		EXPECT_FALSE(evaluate(cond, { { "Shading", "Forward" }, { "Shadows", "On" } }));
		EXPECT_TRUE(evaluate(Condition::always(), {}));
	}

	TEST(ConditionTest, AnyAllNone) {
		for (const auto* a : { "X", "Y" }) {
			for (const auto* b : { "P", "Q" }) {
				const ToggleSet set { { "A", a }, { "B", b } };
				const auto ea = (String { a } == "X");
				const auto eb = (String { b } == "P");

				const auto ca = Condition::equals("A", "X");
				const auto cb = Condition::equals("B", "P");

				EXPECT_EQ(evaluate(Condition::any({ ca, cb }), set), ea || eb);
				EXPECT_EQ(evaluate(Condition::all({ ca, cb }), set), ea && eb);
				EXPECT_EQ(evaluate(Condition::none({ ca, cb }), set), !(ea || eb));
			}
		}
	}

	TEST(ExhaustivenessTest, ReportsMissingCombination) {
		const Vector<Condition> arms { Condition::equals("Shading", "Forward") };

		const auto one = findUncovered({ toggle("Shading", { "Forward", "Deferred" }) }, arms);
		ASSERT_TRUE(one.has_value());
		EXPECT_EQ(*one, (ToggleSet { { "Shading", "Deferred" } }));

		const auto two = findUncovered(
			{ toggle("Shading", { "Forward", "Deferred" }), toggle("Shadows", { "Off", "On" }) },
			arms
		);
		ASSERT_TRUE(two.has_value());
		EXPECT_EQ(*two, (ToggleSet { { "Shading", "Deferred" }, { "Shadows", "Off" } }));
	}

	TEST(ExhaustivenessTest, CompleteCoverageReturnsNone) {
		const Vector<Condition> arms {
			Condition::equals("Shading", "Forward"),
			Condition::negate(Condition::equals("Shading", "Forward"))
		};
		EXPECT_FALSE(findUncovered({ toggle("Shading", { "Forward", "Deferred" }) }, arms).has_value());
	}

	TEST(ModuleTest, CopyIsIndependent) {
		Module mod { .name = "m", .entry = EntryKind::eFragment };
		mod.bindings.emplace_back(BindingDecl { .localName = "a" });

		auto copy = mod;
		copy.bindings.emplace_back(BindingDecl { .localName = "b" });
		copy.bindings.front().localName = "changed";

		ASSERT_EQ(mod.bindings.size(), 1uLL);
		EXPECT_EQ(mod.bindings.front().localName, "a");
	}

	TEST(OpTest, SpellingRoundTrips) {
		for (auto raw = static_cast<u8>(Op::eAdd); raw <= static_cast<u8>(Op::eBitNot); ++raw) {
			const auto op = static_cast<Op>(raw);
			ASSERT_FALSE(spelling(op).empty()) << raw;
			/* `-` reads as eSub; the parser picks eNeg for the prefix form */
			EXPECT_EQ(opOf(spelling(op)), op == Op::eNeg ? Op::eSub : op) << spelling(op);
		}
		EXPECT_TRUE(spelling(Op::eNone).empty());
		EXPECT_EQ(opOf("<=>"), Op::eNone);
		EXPECT_EQ(opOf(""), Op::eNone);
	}

	TEST(LiteralTest, IntegerLiteral) {
		const auto literal = [](LiteralValue value_) {
			Expr e {};
			e.kind = ExprKind::eLiteral;
			e.literal = value_;
			return integerLiteral(e);
		};
		EXPECT_EQ(literal({ s64 { -8 } }).value_or(0LL), -8LL);
		EXPECT_EQ(literal({ u64 { 8uLL } }).value_or(0LL), 8LL);
		EXPECT_EQ(literal({ static_cast<u64>(std::numeric_limits<s64>::max()) }).value_or(0LL), std::numeric_limits<s64>::max());
		EXPECT_FALSE(literal({ static_cast<u64>(std::numeric_limits<s64>::max()) + 1uLL }).has_value());
		EXPECT_FALSE(literal({ 8.0 }).has_value());
		EXPECT_FALSE(literal({ true }).has_value());
	}
}
