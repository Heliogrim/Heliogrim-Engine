#include "pch.h"

/**/

import Engine.Diagnostics;

using namespace hg::engine::diag;
using namespace hg;

namespace HgDiag {
	namespace {
		inline constexpr DiagnosticCodeInfo testTable[] = {
			{ { "TST", 1 }, "first-kind", "first" },
			{ { "TST", 2 }, "second-kind", "second" }
		};

		[[nodiscard]] Diagnostic make() {
			return Diagnostic::error(findCode(testTable, "first-kind"), "msg");
		}

		[[nodiscard]] Result<int, Diagnostic> fail() {
			return Diagnostic::error(findCode(testTable, "first-kind"), "direct").at(SourceSpan { "m", 1, 2, 1, 1 });
		}

		[[nodiscard]] Result<int, Diagnostic> wrap(Result<int, Diagnostic> inner_) {
			if (not inner_) {
				return propagate(std::move(inner_), "while {}", "x");
			}
			return *inner_ + 1;
		}

		[[nodiscard]] Result<int, Diagnostic> forward(Result<int, Diagnostic> inner_) {
			if (not inner_) {
				return propagate(std::move(inner_));
			}
			return *inner_;
		}

		[[nodiscard]] Result<int, Diagnostics> wrapList(Result<int, Diagnostics> inner_) {
			if (not inner_) {
				return propagate(std::move(inner_), "while {}", "x");
			}
			return *inner_;
		}
	}

	static_assert(sizeof(Diagnostic) == sizeof(void*));

	TEST(Diagnostic, SizeIsOnePointer) {
		EXPECT_EQ(sizeof(Diagnostic), sizeof(void*));
	}

	TEST(Diagnostic, BuilderFillsData) {
		const Diagnostic d = Diagnostic::error(findCode(testTable, "first-kind"), "msg")
		                     .at(SourceSpan { "m", 3, 5, 1, 4 })
		                     .label(SourceSpan { "m", 7, 8, 1, 8 }, "here")
		                     .note("n")
		                     .fix(Fix { "f", { TextEdit { SourceSpan { "m", 3, 5, 1, 4 }, "x" } }, Applicability::eMachineApplicable });

		EXPECT_EQ(d.code()->code.number, 1);
		EXPECT_EQ(d.severity(), Severity::eError);
		EXPECT_EQ(d.message(), "msg");
		EXPECT_TRUE(d.primary().is<SourceSpan>());
		ASSERT_EQ(d.labels().size(), 1uLL);
		EXPECT_EQ(d.labels()[0].message, "here");
		ASSERT_EQ(d.notes().size(), 1uLL);
		EXPECT_EQ(d.notes()[0], "n");
		ASSERT_EQ(d.fixes().size(), 1uLL);
		EXPECT_EQ(d.fixes()[0].edits[0].replacement, "x");
	}

	TEST(Diagnostic, MovedFromIsEmpty) {
		auto d = make();
		EXPECT_FALSE(d.empty());

		const auto other = std::move(d);
		EXPECT_TRUE(d.empty());
		EXPECT_FALSE(other.empty());
	}

	TEST(Diagnostic, CopyIsDeep) {
		const auto original = make();

		auto copy = original;
		copy.addNote("extra");

		EXPECT_EQ(original.notes().size(), 0uLL);
		EXPECT_EQ(copy.notes().size(), 1uLL);
	}

	TEST(Diagnostic, CopyAssignIsDeep) {
		const auto original = make();

		Diagnostic copy = Diagnostic::error(findCode(testTable, "second-kind"), "other");
		copy = original;
		copy.addNote("extra");

		EXPECT_EQ(copy.code()->code.number, 1);
		EXPECT_EQ(copy.message(), "msg");
		EXPECT_EQ(original.notes().size(), 0uLL);
		EXPECT_EQ(copy.notes().size(), 1uLL);
	}

	TEST(Diagnostic, CopyAssignSelf) {
		auto d = make();
		const auto& alias = d;

		d = alias;

		ASSERT_FALSE(d.empty());
		EXPECT_EQ(d.message(), "msg");
	}

	TEST(Diagnostic, CopyAssignFromEmpty) {
		auto source = make();
		const auto moved = std::move(source);
		const Diagnostic empty {};

		auto target = make();
		target = empty;
		EXPECT_TRUE(target.empty());

		auto other = make();
		other = source;
		EXPECT_TRUE(other.empty());
		EXPECT_FALSE(moved.empty());
	}

	TEST(Propagate, ForwardsErrorUnchanged) {
		const auto out = forward(Result<int, Diagnostic> { Diagnostic::error(findCode(testTable, "first-kind"), "msg") });

		ASSERT_FALSE(out);
		EXPECT_EQ(out.error().message(), "msg");
		EXPECT_TRUE(out.error().notes().empty());
	}

	TEST(Propagate, ForwardsErrorWithNote) {
		const auto out = wrap(Result<int, Diagnostic> { Diagnostic::error(findCode(testTable, "first-kind"), "msg") });

		ASSERT_FALSE(out);
		EXPECT_EQ(out.error().message(), "msg");
		ASSERT_EQ(out.error().notes().size(), 1uLL);
		EXPECT_EQ(out.error().notes()[0], "while x");
	}

	TEST(Propagate, ListAppendsToLast) {
		Diagnostics list {};
		list.push_back(make());
		list.push_back(make());

		const auto out = wrapList(Result<int, Diagnostics> { Unexpected<Diagnostics> { std::move(list) } });

		ASSERT_FALSE(out);
		ASSERT_EQ(out.error().size(), 2uLL);
		EXPECT_EQ(out.error()[0].notes().size(), 0uLL);
		ASSERT_EQ(out.error()[1].notes().size(), 1uLL);
		EXPECT_EQ(out.error()[1].notes()[0], "while x");
	}

	TEST(Propagate, SuccessPathUntouched) {
		const auto out = wrap(Result<int, Diagnostic> { 7 });

		ASSERT_TRUE(out);
		EXPECT_EQ(*out, 8);
	}

	TEST(Diagnostic, BuilderReturnsAsUnexpected) {
		const auto out = fail();
		ASSERT_FALSE(out);
		EXPECT_EQ(out.error().message(), "direct");
	}

	TEST(Diagnostic, FactoriesSetSeverity) {
		const auto& c = findCode(testTable, "first-kind");
		const Diagnostic w = Diagnostic::warning(c, "w");
		const Diagnostic n = Diagnostic::note(c, "n");
		const Diagnostic h = Diagnostic::help(c, "h");
		EXPECT_EQ(w.severity(), Severity::eWarning);
		EXPECT_EQ(n.severity(), Severity::eNote);
		EXPECT_EQ(h.severity(), Severity::eHelp);
	}
}
