#pragma once

#include <expected>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

#include "Diagnostic.hpp"

namespace hg::engine::accel::lang::ir {
	enum class TokenKind : u8 {
		eEof,
		eIdentifier,
		eInteger,
		eFloat,
		/** text excludes the quotes; no escape sequences are supported */
		eString,

		/** the `#include` directive itself; always followed by eIncludeTarget (or a diagnostic) */
		eInclude,
		/** text between `<` and `>` of an include directive, without the brackets */
		eIncludeTarget,

		eKwFn,
		eKwStruct,
		eKwBlock,
		eKwToggle,
		/** also what follows `@` in `@when` */
		eKwWhen,
		eKwReturn,
		eKwIf,
		eKwElse,
		eKwLet,
		eKwImplicit,
		eKwMut,
		eKwConst,

		eLBrace,
		eRBrace,
		eLParen,
		eRParen,
		eLBracket,
		eRBracket,
		eSemicolon,
		eComma,
		eColon,
		eDot,
		eAt,
		eAssign,
		eArrow,
		eAndAnd,
		eOrOr,
		eBang,
		eEqEq,
		eNotEq,
		eLess,
		eLessEq,
		eGreater,
		eGreaterEq,
		ePlus,
		eMinus,
		eStar,
		eSlash,
		ePercent,
		eAmp,
		ePipe
	};

	/**
	 * `text` views into the lexed source; the caller must keep the source buffer alive as long as the tokens are used.
	 * The final token is always eEof (empty text).
	 */
	struct Token {
		TokenKind kind = TokenKind::eEof;
		StringView text;
		SourceLoc loc;
	};

	/**
	 * Lexes the whole source without copying it. Continues after lexical errors and reports all of them.
	 */
	[[nodiscard]] std::expected<Vector<Token>, Diagnostics> lex(StringView source_);
}
