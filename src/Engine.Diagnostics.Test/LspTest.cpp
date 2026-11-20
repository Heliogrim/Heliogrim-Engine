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

		constexpr auto base = codeDocsBase;
	}

	TEST(Lsp, ErrorWithLabel) {
		const Diagnostic d = Diagnostic::error(findCode(testTable, "first-kind"), "msg")
		                     .at(SourceSpan { "a.asl", 100, 103, 12, 9 })
		                     .label(SourceSpan { "b.asl", 110, 111, 3, 14 }, "lbl")
		                     .note("n");
		EXPECT_EQ(
			toLspJson(d),
			String { R"({"range":{"start":{"line":11,"character":8},"end":{"line":11,"character":11}},)" } +
			R"("severity":1,"code":"TST0001","codeDescription":{"href":")" + String { base } + R"(tst/0001"},)" +
			R"("source":"tst","message":"msg\nn",)" +
			R"("relatedInformation":[{"location":{"uri":"b.asl","range":{"start":{"line":2,"character":13},"end":{"line":2,"character":14}}},"message":"lbl"}]})"
		);
	}

	TEST(Lsp, SeverityMapping) {
		const auto& c = findCode(testTable, "first-kind");
		const auto sev = [&](Diagnostic d) {
			const auto j = toLspJson(d);
			const auto at = j.find("\"severity\":");
			return j[at + 11];
		};
		EXPECT_EQ(sev(Diagnostic::help(c, "m")), '4');
		EXPECT_EQ(sev(Diagnostic::note(c, "m")), '3');
		EXPECT_EQ(sev(Diagnostic::warning(c, "m")), '2');
		EXPECT_EQ(sev(Diagnostic::error(c, "m")), '1');
	}

	TEST(Lsp, FixToCodeAction) {
		const auto& c = findCode(testTable, "first-kind");
		const Diagnostic d = Diagnostic::warning(c, "m")
		                     .fix(Fix { "do", { TextEdit { SourceSpan { "a.asl", 4, 6, 1, 5 }, "x" } }, Applicability::eMachineApplicable })
		                     .fix(Fix { "maybe", {}, Applicability::eMaybeIncorrect });
		const auto json = toLspJson(d);
		EXPECT_EQ(
			toLspCodeActionsJson(d),
			String { R"([{"title":"do","kind":"quickfix","diagnostics":[)" } + json + R"(],"isPreferred":true,)" +
			R"("edit":{"changes":{"a.asl":[{"range":{"start":{"line":0,"character":4},"end":{"line":0,"character":6}},"newText":"x"}]}}},)" +
			R"({"title":"maybe","kind":"quickfix","diagnostics":[)" + json + R"(],"isPreferred":false,"edit":{"changes":{}}}])"
		);
		EXPECT_EQ(toLspCodeActionsJson(Diagnostic::error(c, "m")), "[]");
	}

	TEST(Lsp, EscapesMessage) {
		const Diagnostic d = Diagnostic::error(findCode(testTable, "first-kind"), "a\"b\\c\nd\te\x01");
		EXPECT_NE(toLspJson(d).find(R"("message":"a\"b\\c\nd\te\u0001")"), String::npos);
	}

	TEST(Lsp, PointSpan) {
		const Diagnostic d = Diagnostic::error(findCode(testTable, "first-kind"), "m").at(SourceSpan { "a.asl", 5, 5, 2, 3 });
		EXPECT_EQ(toLspJson(d).rfind(R"({"range":{"start":{"line":1,"character":2},"end":{"line":1,"character":2}},)", 0), 0uLL);
	}

	TEST(Lsp, AssetPrimary) {
		const Diagnostic d = Diagnostic::error(findCode(testTable, "first-kind"), "m").at(AssetPath { "asset://x", "" })
		                     .label(SourceSpan { "u", 0, 1, 1, 1 }, "l");
		const auto j = toLspJson(d);
		EXPECT_EQ(lspUriOf(d), "asset://x");
		EXPECT_EQ(lspUriOf(Diagnostic::error(findCode(testTable, "first-kind"), "m").at(ByteOffset { "blob://b", 1, 2 })), "blob://b");
		EXPECT_EQ(j.rfind(R"({"range":{"start":{"line":0,"character":0},"end":{"line":0,"character":0}},)", 0), 0uLL);
		const Diagnostic e = Diagnostic::error(findCode(testTable, "first-kind"), "m");
		EXPECT_EQ(toLspJson(e).rfind(R"({"range":{"start":{"line":0,"character":0},"end":{"line":0,"character":0}},)", 0), 0uLL);
	}
}
