#include "pch.h"

#include <Engine.Accel.Lang/Ir/Condition.hpp>
#include <Engine.Accel.Lang/Ir/Module.hpp>

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
		Module module { .name = "m", .entry = EntryKind::eFragment };
		module.bindings.emplace_back(BindingDecl { .localName = "a" });

		auto copy = module;
		copy.bindings.emplace_back(BindingDecl { .localName = "b" });
		copy.bindings.front().localName = "changed";

		ASSERT_EQ(module.bindings.size(), 1uLL);
		EXPECT_EQ(module.bindings.front().localName, "a");
	}
}
