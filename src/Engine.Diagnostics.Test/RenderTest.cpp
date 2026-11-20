#include "pch.h"

/**/

import Engine.Diagnostics;

using namespace hg::engine::diag;
using namespace hg;

namespace HgDiag {
	namespace {
		inline constexpr DiagnosticCodeInfo testTable[] = {
			{ { { "TST" }, 1 }, "first-kind", "first" }
		};

		class FixedSource final : public SourceLookup {
		public:
			String text;

			[[nodiscard]] Optional<StringView> find(StringView file_) const override {
				if (file_ == "shader/forward.asl") {
					return StringView { text };
				}
				return {};
			}
		};

		[[nodiscard]] Diagnostic makeSpan() {
			return Diagnostic::error(findCode(testTable, "first-kind"), "msg")
			       .at(SourceSpan { "shader/forward.asl", 100, 103, 12, 9 })
			       .label(SourceSpan { "shader/forward.asl", 110, 111, 12, 14 }, "overlapping argument")
			       .note("n")
			       .fix(Fix { "f", {}, Applicability::eMachineApplicable });
		}

		[[nodiscard]] String twelveLines() {
			String out {};
			for (int i = 1; i <= 12; ++i) {
				out += (i == 12) ? "abcdefg XYZ klm\r\n" : "line\n";
			}
			return out;
		}
	}

	TEST(Render, NoSource) {
		EXPECT_EQ(
			render(makeSpan()),
			R"(error[TST0001]: msg
  --> shader/forward.asl:12:9
  --> shader/forward.asl:12:14: overlapping argument
  = note: n
  = help: f
)"
		);
	}

	TEST(Render, WithSourceSnippet) {
		FixedSource src {};
		src.text = twelveLines();

		EXPECT_EQ(
			render(makeSpan(), &src),
			R"(error[TST0001]: msg
  --> shader/forward.asl:12:9
   |
12 | abcdefg XYZ klm
   |         ^^^
  --> shader/forward.asl:12:14: overlapping argument
   |
12 | abcdefg XYZ klm
   |              ^
  = note: n
  = help: f
)"
		);
	}

	TEST(Render, EachLocationKind) {
		const auto& code = findCode(testTable, "first-kind");

		EXPECT_EQ(render(Diagnostic::warning(code, "m")), "warning[TST0001]: m\n");
		EXPECT_EQ(
			render(Diagnostic::error(code, "m").at(AssetPath { "a/b", "r.x" })),
			"error[TST0001]: m\n  --> asset a/b#r.x\n"
		);
		EXPECT_EQ(
			render(Diagnostic::error(code, "m").at(AssetPath { "a/b", "" })),
			"error[TST0001]: m\n  --> asset a/b\n"
		);
		EXPECT_EQ(
			render(Diagnostic::error(code, "m").at(ActorPath { 7, "c/d" })),
			"error[TST0001]: m\n  --> actor 7/c/d\n"
		);
		EXPECT_EQ(
			render(Diagnostic::note(code, "m").at(ByteOffset { "blob.bin", 16, 4 })),
			"note[TST0001]: m\n  --> blob blob.bin @ 16+4\n"
		);
	}

	TEST(Render, NoLocationLabelKeepsMessage) {
		EXPECT_EQ(
			render(Diagnostic::error(findCode(testTable, "first-kind"), "m").label(NoLocation {}, "why").note("n")),
			"error[TST0001]: m\n  = why\n  = note: n\n"
		);
	}

	TEST(Render, SpanBeyondSourceFallsBack) {
		FixedSource src {};
		src.text = "one\ntwo\n";

		EXPECT_EQ(render(makeSpan(), &src), render(makeSpan()));
	}
}
