#include "Lexer.hpp"

#include <array>
#include <string>

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	[[nodiscard]] bool isAlpha(char c_) noexcept {
		return (c_ >= 'a' && c_ <= 'z') || (c_ >= 'A' && c_ <= 'Z') || c_ == '_';
	}

	[[nodiscard]] bool isDigit(char c_) noexcept {
		return c_ >= '0' && c_ <= '9';
	}

	struct Keyword {
		StringView text;
		ir::TokenKind kind;
	};

	constexpr std::array<Keyword, 10> keywords {
		{
			{ "fn", ir::TokenKind::eKwFn }, { "struct", ir::TokenKind::eKwStruct },
			{ "block", ir::TokenKind::eKwBlock }, { "toggle", ir::TokenKind::eKwToggle },
			{ "when", ir::TokenKind::eKwWhen }, { "return", ir::TokenKind::eKwReturn },
			{ "if", ir::TokenKind::eKwIf }, { "else", ir::TokenKind::eKwElse },
			{ "let", ir::TokenKind::eKwLet }, { "implicit", ir::TokenKind::eKwImplicit }
		}
	};

	class Lexer {
	public:
		explicit Lexer(StringView source_) :
			_src(source_) {}

		[[nodiscard]] std::expected<Vector<ir::Token>, ir::Diagnostics> run() {
			while (skipTrivia(), _pos < _src.size()) {
				next();
			}
			_tokens.emplace_back(ir::Token { ir::TokenKind::eEof, _src.substr(_src.size()), here() });

			if (not _diags.empty()) {
				return std::unexpected(std::move(_diags));
			}
			return std::move(_tokens);
		}

	private:
		StringView _src;
		size_t _pos = 0uz;
		ir::SourceLoc _loc {};
		Vector<ir::Token> _tokens;
		ir::Diagnostics _diags;

		[[nodiscard]] ir::SourceLoc here() const noexcept {
			return _loc;
		}

		[[nodiscard]] char peek(size_t off_ = 0uz) const noexcept {
			return _pos + off_ < _src.size() ? _src[_pos + off_] : '\0';
		}

		void advance() noexcept {
			if (_src[_pos++] == '\n') {
				++_loc.line;
				_loc.column = 1uL;
			} else {
				++_loc.column;
			}
		}

		void error(ir::SourceLoc at_, String msg_) {
			_diags.emplace_back(ir::Diagnostic { ir::Severity::eError, std::move(msg_), at_.line, at_.column });
		}

		void emit(ir::TokenKind kind_, size_t begin_, ir::SourceLoc at_) {
			_tokens.emplace_back(ir::Token { kind_, _src.substr(begin_, _pos - begin_), at_ });
		}

		void skipTrivia() {
			while (_pos < _src.size()) {
				const char c = peek();
				if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
					advance();
				} else if (c == '/' && peek(1) == '/') {
					while (_pos < _src.size() && peek() != '\n') {
						advance();
					}
				} else if (c == '/' && peek(1) == '*') {
					const auto at = here();
					advance();
					advance();
					while (_pos < _src.size() && not(peek() == '*' && peek(1) == '/')) {
						advance();
					}
					if (_pos >= _src.size()) {
						error(at, "Unterminated block comment.");
					} else {
						advance();
						advance();
					}
				} else {
					return;
				}
			}
		}

		void next() {
			const char c = peek();
			const auto at = here();
			const auto begin = _pos;

			if (isAlpha(c)) {
				while (isAlpha(peek()) || isDigit(peek())) {
					advance();
				}
				auto kind = ir::TokenKind::eIdentifier;
				for (const auto& kw : keywords) {
					if (kw.text == _src.substr(begin, _pos - begin)) {
						kind = kw.kind;
					}
				}
				return emit(kind, begin, at);
			}
			if (isDigit(c)) {
				return number();
			}
			if (c == '"') {
				return string();
			}
			if (c == '#') {
				return directive();
			}

			// two-character operators first
			struct Pair {
				char a, b;
				ir::TokenKind kind;
			};
			static constexpr std::array<Pair, 6> pairs {
				{
					{ '-', '>', ir::TokenKind::eArrow }, { '&', '&', ir::TokenKind::eAndAnd },
					{ '|', '|', ir::TokenKind::eOrOr }, { '=', '=', ir::TokenKind::eEqEq },
					{ '!', '=', ir::TokenKind::eNotEq }, { '<', '=', ir::TokenKind::eLessEq }
				}
			};
			for (const auto& p : pairs) {
				if (c == p.a && peek(1) == p.b) {
					advance();
					advance();
					return emit(p.kind, begin, at);
				}
			}
			if (c == '>' && peek(1) == '=') {
				advance();
				advance();
				return emit(ir::TokenKind::eGreaterEq, begin, at);
			}

			ir::TokenKind kind;
			switch (c) {
				case '{': kind = ir::TokenKind::eLBrace; break;
				case '}': kind = ir::TokenKind::eRBrace; break;
				case '(': kind = ir::TokenKind::eLParen; break;
				case ')': kind = ir::TokenKind::eRParen; break;
				case '[': kind = ir::TokenKind::eLBracket; break;
				case ']': kind = ir::TokenKind::eRBracket; break;
				case ';': kind = ir::TokenKind::eSemicolon; break;
				case ',': kind = ir::TokenKind::eComma; break;
				case ':': kind = ir::TokenKind::eColon; break;
				case '.': kind = ir::TokenKind::eDot; break;
				case '@': kind = ir::TokenKind::eAt; break;
				case '=': kind = ir::TokenKind::eAssign; break;
				case '!': kind = ir::TokenKind::eBang; break;
				case '<': kind = ir::TokenKind::eLess; break;
				case '>': kind = ir::TokenKind::eGreater; break;
				case '+': kind = ir::TokenKind::ePlus; break;
				case '-': kind = ir::TokenKind::eMinus; break;
				case '*': kind = ir::TokenKind::eStar; break;
				case '/': kind = ir::TokenKind::eSlash; break;
				case '%': kind = ir::TokenKind::ePercent; break;
				case '&': kind = ir::TokenKind::eAmp; break;
				case '|': kind = ir::TokenKind::ePipe; break;
				default: {
					advance();
					error(at, std::string("Unexpected character '") + c + "'.");
					return;
				}
			}
			advance();
			emit(kind, begin, at);
		}

		void number() {
			const auto at = here();
			const auto begin = _pos;
			bool isFloat = false;

			while (isDigit(peek())) {
				advance();
			}
			if (peek() == '.' && isDigit(peek(1))) {
				isFloat = true;
				advance();
				while (isDigit(peek())) {
					advance();
				}
			}
			if (peek() == 'e' || peek() == 'E') {
				const size_t signLen = (peek(1) == '+' || peek(1) == '-') ? 1uz : 0uz;
				if (isDigit(peek(1 + signLen))) {
					isFloat = true;
					for (size_t i = 0uz; i <= signLen; ++i) {
						advance();
					}
					while (isDigit(peek())) {
						advance();
					}
				}
			}

			// a literal running into letters or another '.' is malformed; swallow the whole run
			if (isAlpha(peek()) || (peek() == '.' && isDigit(peek(1)))) {
				while (isAlpha(peek()) || isDigit(peek()) || (peek() == '.' && isDigit(peek(1)))) {
					advance();
				}
				error(at, "Malformed number literal '" + String(_src.substr(begin, _pos - begin)) + "'.");
				return;
			}
			emit(isFloat ? ir::TokenKind::eFloat : ir::TokenKind::eInteger, begin, at);
		}

		void string() {
			const auto at = here();
			advance();// opening quote
			const auto begin = _pos;
			while (_pos < _src.size() && peek() != '"' && peek() != '\n') {
				advance();
			}
			if (peek() != '"') {
				error(at, "Unterminated string literal.");
				return;
			}
			const auto end = _pos;
			const auto textLoc = at;
			advance();// closing quote
			_tokens.emplace_back(ir::Token { ir::TokenKind::eString, _src.substr(begin, end - begin), textLoc });
		}

		void directive() {
			const auto at = here();
			const auto begin = _pos;
			advance();// '#'
			while (isAlpha(peek())) {
				advance();
			}
			if (_src.substr(begin, _pos - begin) != "#include") {
				error(at, "Unknown directive '" + String(_src.substr(begin, _pos - begin)) + "'.");
				return;
			}
			emit(ir::TokenKind::eInclude, begin, at);

			while (peek() == ' ' || peek() == '\t') {
				advance();
			}
			const auto openAt = here();
			if (peek() != '<') {
				error(openAt, "Expected '<' after #include.");
				return;
			}
			advance();
			const auto targetAt = here();
			const auto targetBegin = _pos;
			while (_pos < _src.size() && peek() != '>' && peek() != '\n') {
				advance();
			}
			if (peek() != '>') {
				error(openAt, "Unterminated #include target, expected '>'.");
				return;
			}
			emit(ir::TokenKind::eIncludeTarget, targetBegin, targetAt);
			advance();// '>'
		}
	};
}

std::expected<Vector<ir::Token>, ir::Diagnostics> ir::lex(StringView source_) {
	return Lexer { source_ }.run();
}
