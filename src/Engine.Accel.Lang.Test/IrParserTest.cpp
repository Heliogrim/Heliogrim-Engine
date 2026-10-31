#include "pch.h"

#include <array>

#include <Engine.Accel.Lang/Ir/Lexer.hpp>

using namespace hg;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	namespace {
		[[nodiscard]] Vector<Token> lexOk(StringView source_) {
			auto res = lex(source_);
			EXPECT_TRUE(res.has_value());
			return res.has_value() ? std::move(res).value() : Vector<Token> {};
		}

		[[nodiscard]] Diagnostics lexFail(StringView source_) {
			auto res = lex(source_);
			EXPECT_FALSE(res.has_value());
			return res.has_value() ? Diagnostics {} : std::move(res).error();
		}
	}

	TEST(LexerTest, TracksLineAndColumn) {
		const auto tokens = lexOk("a\n\n\tbc");
		ASSERT_EQ(tokens.size(), 3uz);// a, bc, eof
		EXPECT_EQ(tokens[0].loc.line, 1u);
		EXPECT_EQ(tokens[0].loc.column, 1u);
		EXPECT_EQ(tokens[1].text, "bc");
		EXPECT_EQ(tokens[1].loc.line, 3u);
		EXPECT_EQ(tokens[1].loc.column, 2u);// tab is one column
		EXPECT_EQ(tokens[2].kind, TokenKind::eEof);
	}

	TEST(LexerTest, SkipsComments) {
		const auto tokens = lexOk("a // line\n/* block\n x */ b");
		ASSERT_EQ(tokens.size(), 3uz);
		EXPECT_EQ(tokens[0].text, "a");
		EXPECT_EQ(tokens[1].text, "b");
		EXPECT_EQ(tokens[1].loc.line, 3u);
		EXPECT_EQ(tokens[1].loc.column, 7u);
	}

	TEST(LexerTest, UnterminatedBlockCommentIsDiagnostic) {
		const auto diags = lexFail("a\n  /* never closed");
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(diags[0].severity, Severity::eError);
		EXPECT_EQ(diags[0].line, 2u);
		EXPECT_EQ(diags[0].column, 3u);
	}

	TEST(LexerTest, UnterminatedStringIsDiagnostic) {
		const auto diags = lexFail("x = \"abc\ny");
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(diags[0].line, 1u);
		EXPECT_EQ(diags[0].column, 5u);
	}

	TEST(LexerTest, KeywordsAreDistinctFromIdentifiers) {
		const auto tokens = lexOk("fn struct block toggle when return if else let implicit fnx Let");
		const std::array<TokenKind, 13> expected {
			TokenKind::eKwFn, TokenKind::eKwStruct, TokenKind::eKwBlock, TokenKind::eKwToggle, TokenKind::eKwWhen,
			TokenKind::eKwReturn, TokenKind::eKwIf, TokenKind::eKwElse, TokenKind::eKwLet, TokenKind::eKwImplicit,
			TokenKind::eIdentifier, TokenKind::eIdentifier, TokenKind::eEof
		};
		ASSERT_EQ(tokens.size(), expected.size());
		for (auto i = 0uz; i < tokens.size(); ++i) {
			EXPECT_EQ(tokens[i].kind, expected[i]) << i;
		}
	}

	TEST(LexerTest, MultiCharPunctuationIsOneToken) {
		const auto tokens = lexOk("-> && || == != <= >= - & ! @when");
		const std::array<TokenKind, 13> expected {
			TokenKind::eArrow, TokenKind::eAndAnd, TokenKind::eOrOr, TokenKind::eEqEq, TokenKind::eNotEq,
			TokenKind::eLessEq, TokenKind::eGreaterEq, TokenKind::eMinus, TokenKind::eAmp, TokenKind::eBang,
			TokenKind::eAt, TokenKind::eKwWhen, TokenKind::eEof
		};
		ASSERT_EQ(tokens.size(), expected.size());
		for (auto i = 0uz; i < tokens.size(); ++i) {
			EXPECT_EQ(tokens[i].kind, expected[i]) << i;
		}
	}

	TEST(LexerTest, NumbersAndStrings) {
		const auto tokens = lexOk("12 3.5 1e3 \"a b\"");
		ASSERT_EQ(tokens.size(), 5uz);
		EXPECT_EQ(tokens[0].kind, TokenKind::eInteger);
		EXPECT_EQ(tokens[1].kind, TokenKind::eFloat);
		EXPECT_EQ(tokens[1].text, "3.5");
		EXPECT_EQ(tokens[2].kind, TokenKind::eFloat);
		EXPECT_EQ(tokens[3].kind, TokenKind::eString);
		EXPECT_EQ(tokens[3].text, "a b");// without quotes
	}

	TEST(LexerTest, MalformedNumberIsDiagnostic) {
		const auto diags = lexFail("1.2.3 12ab 1e");
		ASSERT_EQ(diags.size(), 3uz);
		EXPECT_EQ(diags[0].column, 1u);
		EXPECT_EQ(diags[1].column, 7u);
		EXPECT_EQ(diags[2].column, 12u);
	}

	TEST(LexerTest, IncludeDirectiveIsDirectiveThenTarget) {
		const auto tokens = lexOk("#include <Common/View>\nfn");
		ASSERT_EQ(tokens.size(), 4uz);
		EXPECT_EQ(tokens[0].kind, TokenKind::eInclude);
		EXPECT_EQ(tokens[1].kind, TokenKind::eIncludeTarget);
		EXPECT_EQ(tokens[1].text, "Common/View");
		EXPECT_EQ(tokens[1].loc.column, 11u);
		EXPECT_EQ(tokens[2].kind, TokenKind::eKwFn);
		EXPECT_EQ(tokens[2].loc.line, 2u);
	}

	TEST(LexerTest, BadIncludeIsDiagnostic) {
		EXPECT_EQ(lexFail("#include foo").size(), 1uz);
		EXPECT_EQ(lexFail("#include <foo").size(), 1uz);
		EXPECT_EQ(lexFail("#define x").size(), 1uz);
	}

	TEST(LexerTest, UnknownCharacterIsLocatedAndLexingContinues) {
		const auto diags = lexFail("a\n b $ c ` d");
		ASSERT_EQ(diags.size(), 2uz);
		EXPECT_EQ(diags[0].line, 2u);
		EXPECT_EQ(diags[0].column, 4u);
		EXPECT_EQ(diags[1].column, 8u);
	}
}
