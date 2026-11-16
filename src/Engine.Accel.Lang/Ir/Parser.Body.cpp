#include <cassert>
#include <charconv>
#include <limits>

#include "Builtin.hpp"
#include "ParserImpl.hpp"
#include "Typecheck.hpp"

using namespace hg::engine::accel::lang::ir::detail;
using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	using ir::findBuiltinFunction;

	[[nodiscard]] bool isBuiltinVariable(StringView name_) {
		return ir::findBuiltinVariable(name_) != nullptr;
	}

	/* binary operator precedence, higher binds tighter (all left-associative); 0 = no binary operator */
	[[nodiscard]] u32 precedenceOf(TokenKind kind_) {
		switch (kind_) {
			case TokenKind::eOrOr: return 1uL;
			case TokenKind::eAndAnd: return 2uL;
			case TokenKind::ePipe: return 3uL;
			case TokenKind::eCaret: return 4uL;
			case TokenKind::eAmp: return 5uL;
			case TokenKind::eEqEq:
			case TokenKind::eNotEq: return 6uL;
			case TokenKind::eLess:
			case TokenKind::eLessEq:
			case TokenKind::eGreater:
			case TokenKind::eGreaterEq: return 7uL;
			case TokenKind::eShl:
			case TokenKind::eShr: return 8uL;
			case TokenKind::ePlus:
			case TokenKind::eMinus: return 9uL;
			case TokenKind::eStar:
			case TokenKind::eSlash:
			case TokenKind::ePercent: return 10uL;
			default: return 0uL;
		}
	}

	/* bool, float or integer (`0x` hex, leading-0 octal, `u` suffix = unsigned) text from the lexer; None when malformed or out of range */
	[[nodiscard]] Opt<ir::LiteralValue> literalOf(StringView text_) {
		if (text_ == "true" || text_ == "false") {
			return ir::LiteralValue { text_ == "true", String { text_ } };
		}

		const auto* first = text_.data();
		const auto* end = first + text_.size();
		const auto hex = text_.size() > 2uz && text_.front() == '0' && (text_[1] == 'x' || text_[1] == 'X');

		if (not hex && text_.find_first_of(".eE") != StringView::npos) {
			f64 value = 0.0;
			const auto res = std::from_chars(first, end, value);
			if (res.ec != std::errc {} || res.ptr != end) {
				return None;
			}
			return ir::LiteralValue { value, String { text_ } };
		}

		const auto isUnsigned = text_.ends_with('u') || text_.ends_with('U');
		const auto* digits = hex ? first + 2 : first;
		const auto* digitsEnd = isUnsigned ? end - 1 : end;
		const auto octal = not hex && text_.size() > 1uz && text_.front() == '0';

		u64 value = 0uL;
		const auto res = std::from_chars(digits, digitsEnd, value, hex ? 16 : (octal ? 8 : 10));
		if (res.ec != std::errc {} || res.ptr != digitsEnd) {
			return None;
		}

		if (isUnsigned || value > static_cast<u64>(std::numeric_limits<s64>::max())) {
			return ir::LiteralValue { value, String { text_ } };
		}
		return ir::LiteralValue { static_cast<s64>(value), String { text_ } };
	}
}

/*
 * Bodies. Every statement recursion passes through statement(), every expression recursion through unary(),
 * both under the shared Nest budget. Recursive frames stay small: node building and diagnostics live in
 * non-recursive helpers. A parser recovers only from errors in its own tokens (fail()); a failed
 * sub-statement has recovered already and is passed up as None.
 */

ir::ExprRef Parser::expr(ir::Expr expr_) {
	auto& exprs = _shared.mod.exprs;
	exprs.emplace_back(std::move(expr_));
	return ir::ExprRef { static_cast<u32>(exprs.size() - 1uz) };
}

ir::ExprRef Parser::node(const ir::ExprKind kind_, const StringView name_, Vector<ir::ExprRef> operands_, ref<const ir::SourceLoc> loc_) {
	ir::Expr out {};
	out.kind = kind_;
	out.name = name_;
	out.operands = std::move(operands_);
	out.loc = loc_;
	return expr(std::move(out));
}

ir::ExprRef Parser::literal(const StringView text_, ref<const ir::Token> at_) {
	const auto out = node(ir::ExprKind::eLiteral, {}, {}, at_.loc);
	if (auto value = literalOf(text_); value.has_value()) {
		_shared.mod.exprs[out.index].literal = std::move(*value);

	} else {
		error(codes::asl("literal-out-of-range"), at_, "literal '" + String { text_ } + "' is malformed or does not fit 64 bits");
	}
	return out;
}

ir::StmtRef Parser::add(ir::Stmt stmt_) {
	auto& stmts = _shared.mod.stmts;
	const auto& exprs = _shared.mod.exprs;

	if (stmt_.lhs.index < exprs.size()) {
		stmt_.loc = exprs[stmt_.lhs.index].loc;

	} else if (_pos > 0uz) {
		stmt_.loc = _tokens[_pos - 1uz].loc;
	}

	stmts.emplace_back(std::move(stmt_));
	return ir::StmtRef { static_cast<u32>(stmts.size() - 1uz) };
}

ir::StmtRef Parser::stmt(const ir::StmtKind kind_, const ir::ExprRef lhs_, Vector<ir::StmtRef> children_) {
	ir::Stmt out {};
	out.kind = kind_;
	out.lhs = lhs_;
	out.children = std::move(children_);
	return add(std::move(out));
}

Opt<ir::StmtRef> Parser::fail() {
	recover();
	return None;
}

void Parser::unterminated(ref<const ir::Token> open_) {
	if (_eofErrors == 0uz && _shared.depthErrors == 0uL) {
		error(codes::asl("unexpected-token"), open_, "unterminated block, expected '}'");
	}
	++_eofErrors;
}

/* Statements */

/* `{ statement* }`; the block is a scope */
ir::StmtRef Parser::block() {
	const auto& open = take();
	const Scope scope { _locals, _scope };

	Vector<ir::StmtRef> children {};
	while (not at(TokenKind::eRBrace) && not at(TokenKind::eEof)) {
		if (const auto child = statement(); child.has_value()) {
			children.emplace_back(*child);
		}
	}

	if (not accept(TokenKind::eRBrace)) {
		unterminated(open);
	}

	return stmt(ir::StmtKind::eBlock, {}, std::move(children));
}

/* consumes at least one token unless at `}` or the end of input */
Opt<ir::StmtRef> Parser::statement() {
	const Nest nest { _shared.depth };
	if (tooDeep(peek())) {
		if (at(TokenKind::eLBrace)) {
			skipGroup(TokenKind::eLBrace, TokenKind::eRBrace);
			return None;
		}
		return fail();
	}

	const auto& head = peek();
	if (head.kind == TokenKind::eLBrace) {
		return SomeVal(block());
	}

	if (accept(TokenKind::eSemicolon)) {
		return SomeVal(stmt(ir::StmtKind::eBlock));
	}

	if (head.kind == TokenKind::eAt) {
		return whenStatement();
	}

	if (head.kind == TokenKind::eKwIf) {
		return ifStatement();
	}

	if (head.kind == TokenKind::eKwReturn) {
		return returnStatement();
	}

	if (head.kind == TokenKind::eKwLet) {
		return letStatement(false);
	}

	if (head.kind == TokenKind::eKwConst) {
		return letStatement(true);
	}

	if (head.kind == TokenKind::eKwFor) {
		return forStatement();
	}

	if (head.kind == TokenKind::eKwWhile) {
		return whileStatement();
	}

	if (head.kind == TokenKind::eKwBreak || head.kind == TokenKind::eKwContinue || head.kind == TokenKind::eKwDiscard) {
		take();
		if (not expect(TokenKind::eSemicolon, "';'")) {
			return fail();
		}

		return SomeVal(
			stmt(
				head.kind == TokenKind::eKwBreak ?
				ir::StmtKind::eBreak :
				(head.kind == TokenKind::eKwContinue ? ir::StmtKind::eContinue : ir::StmtKind::eDiscard)
			)
		);
	}

	if (head.kind == TokenKind::eIdentifier) {
		const auto ahead = next().kind;
		if (ahead == TokenKind::eIdentifier || (ahead == TokenKind::eLBracket && isTypeName(head.text))) {
			return typedDeclaration();
		}
	}

	return simpleStatement();
}

/* the then/else arm of `if` or `@when`: a scope even without braces */
Opt<ir::StmtRef> Parser::branch() {
	const Scope scope { _locals, _scope };
	return statement();
}

/* `@when (cond) statement` */
Opt<ir::StmtRef> Parser::whenStatement() {
	const auto head = whenHead();
	if (not head.has_value()) {
		return fail();
	}

	const auto child = branch();
	if (not child.has_value()) {
		return None;
	}

	_shared.mod.stmts[head->index].children.emplace_back(*child);
	return SomeVal(*head);
}

/* keeps the condition out of the recursive frame */
Opt<ir::StmtRef> Parser::whenHead() {
	take();
	if (not accept(TokenKind::eKwWhen)) {
		error(codes::asl("misplaced-annotation"), peek(), "only @when applies to a statement");
		return None;
	}

	auto cond = parenCondition();
	if (not cond.has_value()) {
		return None;
	}

	ir::Stmt when {};
	when.kind = ir::StmtKind::eWhen;
	when.when = std::move(*cond);
	return SomeVal(add(std::move(when)));
}

/* `if (expr) statement [else statement]` */
Opt<ir::StmtRef> Parser::ifStatement() {
	take();
	if (not expect(TokenKind::eLParen, "'(' after if")) {
		return fail();
	}

	const auto cond = expression();
	if (not cond.has_value() || not expect(TokenKind::eRParen, "')'")) {
		return fail();
	}

	const auto then = branch();
	if (not then.has_value()) {
		return None;
	}

	if (not accept(TokenKind::eKwElse)) {
		return SomeVal(stmt(ir::StmtKind::eIf, *cond, { *then }));
	}
	const auto otherwise = branch();
	if (not otherwise.has_value()) {
		return None;
	}

	return SomeVal(stmt(ir::StmtKind::eIf, *cond, { *then, *otherwise }));
}

Opt<ir::StmtRef> Parser::returnStatement() {
	take();
	if (accept(TokenKind::eSemicolon)) {
		return SomeVal(stmt(ir::StmtKind::eReturn));
	}

	const auto value = expression();
	if (not value.has_value() || not expect(TokenKind::eSemicolon, "';'")) {
		return fail();
	}

	return SomeVal(stmt(ir::StmtKind::eReturn, *value));
}

/* `let|const name [: Type] [= init|&place|&mut place];`, a type or an initializer required */
Opt<ir::StmtRef> Parser::letStatement(bool const_) {
	const auto keyword = take().loc;
	if (not at(TokenKind::eIdentifier)) {
		syntaxError("a local name");
		return fail();
	}

	const auto& name = take();
	Opt<ir::TypeRef> declType = None;
	if (accept(TokenKind::eColon)) {
		declType = type();
		if (not declType.has_value()) {
			return fail();
		}

	} else if (not at(TokenKind::eAssign)) {
		error(
			codes::asl("incomplete-local-declaration"),
			name,
			"'" + String(const_ ? "const" : "let") + "' declaration needs a type or an initializer"
		);
		return fail();
	}

	return local(name, declType.value_or(ir::TypeRef {}), const_, keyword);
}

/* `Type name [= init];` */
Opt<ir::StmtRef> Parser::typedDeclaration() {
	const auto declType = type();
	if (not declType.has_value()) {
		return fail();
	}

	if (not at(TokenKind::eIdentifier)) {
		syntaxError("a local name");
		return fail();
	}

	return local(take(), *declType, false);
}

/* `[= init];` of both declaration forms (type unset = inferred later); the name is in scope after its
 * initializer, even after an error */
Opt<ir::StmtRef> Parser::local(ref<const ir::Token> name_, const ir::TypeRef type_, const bool const_, ir::SourceLoc keyword_) {
	ir::Stmt decl {};
	decl.kind = ir::StmtKind::eDeclareLocal;
	decl.name = name_.text;
	decl.type = type_;
	decl.mutable_ = not const_;
	decl.keyword = ::hg::move(keyword_);

	bool valid = true;
	declaredName(name_, false);
	if (accept(TokenKind::eAssign)) {
		if (accept(TokenKind::eAmp)) {
			decl.kind = ir::StmtKind::eDeclareRef;
			decl.mutable_ = accept(TokenKind::eKwMut);

			if (decl.mutable_ && const_) {
				error(codes::asl("not-assignable"), name_, "cannot take '&mut' into a 'const' alias");
			}

			if (type_.index != ir::TypeRef {}.index) {
				error(codes::asl("typed-reference-declaration"), name_, "a reference declaration cannot have a type annotation");
			}
		}

		const auto init = expression();
		valid = init.has_value();
		decl.lhs = valid ? *init : decl.lhs;
	}
	valid = valid && expect(TokenKind::eSemicolon, "';'");

	if (std::find(_locals.begin() + static_cast<ptrdiff_t>(_scope), _locals.end(), name_.text) != _locals.end()) {
		error(codes::asl("duplicate-declaration"), name_, "duplicate local '" + String(name_.text) + "'");
	}

	_locals.emplace_back(name_.text);
	if (not valid) {
		return fail();
	}

	return SomeVal(add(std::move(decl)));
}

/* `while (cond) statement` */
Opt<ir::StmtRef> Parser::whileStatement() {
	take();
	if (not expect(TokenKind::eLParen, "'(' after while")) {
		return fail();
	}

	const auto cond = expression();
	if (not cond.has_value() || not expect(TokenKind::eRParen, "')'")) {
		return fail();
	}

	const auto body = branch();
	if (not body.has_value()) {
		return None;
	}

	return SomeVal(stmt(ir::StmtKind::eWhile, *cond, { *body }));
}

/* `for (init; cond; step) statement`; init is a full statement, step a simple one, each optional */
Opt<ir::StmtRef> Parser::forStatement() {
	take();
	if (not expect(TokenKind::eLParen, "'(' after for")) {
		return fail();
	}

	const Scope scope { _locals, _scope };
	const auto init = statement();
	if (not init.has_value()) {
		return None;
	}

	auto cond = SomeVal<ir::ExprRef>();
	if (not at(TokenKind::eSemicolon)) {
		cond = expression();
	}

	if (not cond.has_value() || not expect(TokenKind::eSemicolon, "';'")) {
		return fail();
	}

	auto step = SomeVal(stmt(ir::StmtKind::eBlock));
	if (not at(TokenKind::eRParen)) {
		step = simpleStatement(false);
	}

	if (not step.has_value() || not expect(TokenKind::eRParen, "')'")) {
		return fail();
	}

	const auto body = branch();
	if (not body.has_value()) {
		return None;
	}

	return SomeVal(stmt(ir::StmtKind::eFor, *cond, { *init, *body, *step }));
}

/* `expr;`, `lvalue = expr;`, `lvalue op= expr;`, `++lvalue;`, `lvalue++;`; the `;` is left to the caller of a for step */
Opt<ir::StmtRef> Parser::simpleStatement(const bool terminated_) {
	const auto& start = peek();
	const auto prefix = at(TokenKind::ePlusPlus) || at(TokenKind::eMinusMinus);
	const auto prefixToken = prefix ? &take() : nullptr;

	const auto lhs = expression();
	if (not lhs.has_value()) {
		return fail();
	}

	const auto* step = prefixToken;
	if (step == nullptr && (at(TokenKind::ePlusPlus) || at(TokenKind::eMinusMinus))) {
		step = &take();
	}

	if (step == nullptr && not at(TokenKind::eAssign) && not at(TokenKind::eCompoundAssign)) {
		if (terminated_ && not expect(TokenKind::eSemicolon, "';'")) {
			return fail();
		}
		return SomeVal(stmt(ir::StmtKind::eExpr, *lhs));
	}

	ir::Stmt assign {};
	assign.kind = ir::StmtKind::eAssign;
	assign.lhs = *lhs;
	if (not assignable(*lhs)) {
		error(codes::asl("not-assignable"), start, "cannot assign to this expression");
	}

	if (step != nullptr) {
		assign.op = step->kind == TokenKind::ePlusPlus ? ir::Op::eAdd : ir::Op::eSub;
		assign.rhs = literal("1", *step);

	} else {
		if (at(TokenKind::eCompoundAssign)) {
			const auto text = take().text;
			assign.op = ir::opOf(text.substr(0uz, text.size() - 1uz));
			assert(assign.op != ir::Op::eNone);
		} else {
			take();
		}

		const auto rhs = expression();
		if (not rhs.has_value()) {
			return fail();
		}

		assign.rhs = *rhs;
	}

	if (terminated_ && not expect(TokenKind::eSemicolon, "';'")) {
		return fail();
	}
	return SomeVal(add(std::move(assign)));
}

/* a name, optionally behind field and index accesses */
bool Parser::assignable(ir::ExprRef ref_) const {
	return ir::isPlace(_shared.mod, ref_);
}

/* Expressions: `?:` (right-assoc, only at the lowest level) < binary operators (precedence climbing, see precedenceOf) < unary `- ! ~` < postfix `.name [i]` < primary */

Opt<ir::ExprRef> Parser::expression(const u32 minPrecedence_) {
	auto lhs = unary();
	while (lhs.has_value() && precedenceOf(peek().kind) >= minPrecedence_) {
		const auto& op = take();
		const auto rhs = expression(precedenceOf(op.kind) + 1uL);
		if (not rhs.has_value()) {
			lhs = None;
			continue;
		}

		const auto binary = node(ir::ExprKind::eBinary, {}, { *lhs, *rhs }, op.loc);
		_shared.mod.exprs[binary.index].op = ir::opOf(op.text);
		lhs = SomeVal(binary);
	}

	if (minPrecedence_ == 1uL && lhs.has_value() && at(TokenKind::eQuestion)) {
		return ternary(*lhs);
	}
	return lhs;
}

/* at '?' after the condition; the arms recurse, so they share the Nest budget */
Opt<ir::ExprRef> Parser::ternary(ir::ExprRef cond_) {
	const Nest nest { _shared.depth };
	const auto& op = take();
	if (tooDeep(op)) {
		return None;
	}

	const auto then = expression();
	if (not then.has_value() || not expect(TokenKind::eColon, "':' in conditional expression")) {
		return None;
	}

	const auto otherwise = expression();
	if (not otherwise.has_value()) {
		return None;
	}

	return SomeVal(node(ir::ExprKind::eTernary, {}, { cond_, *then, *otherwise }, op.loc));
}

Opt<ir::ExprRef> Parser::unary() {
	const Nest nest { _shared.depth };
	if (tooDeep(peek())) {
		return None;
	}

	if (at(TokenKind::eMinus) || at(TokenKind::eBang) || at(TokenKind::eTilde)) {
		const auto& op = take();
		const auto operand = unary();
		if (not operand.has_value()) {
			return None;
		}

		const auto unary = node(ir::ExprKind::eUnary, {}, { *operand }, op.loc);
		_shared.mod.exprs[unary.index].op = op.kind == TokenKind::eMinus ? ir::Op::eNeg : op.kind == TokenKind::eBang ? ir::Op::eNot : ir::Op::eBitNot;
		return SomeVal(unary);
	}

	return postfix(primary());
}

Opt<ir::ExprRef> Parser::postfix(Opt<ir::ExprRef> base_) {
	while (base_.has_value()) {
		if (accept(TokenKind::eDot)) {
			if (not at(TokenKind::eIdentifier)) {
				syntaxError("a field name");
				return None;
			}
			// any field name is accepted here; typecheck validates struct members and swizzles against the base type
			base_ = SomeVal(node(ir::ExprKind::eField, take().text, { *base_ }, _shared.mod.exprs[base_->index].loc));

		} else if (accept(TokenKind::eLBracket)) {
			const auto index = expression();
			if (not index.has_value()) {
				return None;
			}

			const auto loc = _shared.mod.exprs[base_->index].loc;
			if (accept(TokenKind::eDotDot)) {
				const auto hi = expression();
				if (not hi.has_value() || not expect(TokenKind::eRBracket, "']'")) {
					return None;
				}
				base_ = SomeVal(node(ir::ExprKind::eRange, {}, { *base_, *index, *hi }, loc));

			} else {
				if (not expect(TokenKind::eRBracket, "']'")) {
					return None;
				}
				base_ = SomeVal(node(ir::ExprKind::eIndex, {}, { *base_, *index }, loc));
			}

		} else {
			break;
		}
	}
	return base_;
}

Opt<ir::ExprRef> Parser::primary() {
	const auto& token = peek();
	if (token.kind == TokenKind::eInteger || token.kind == TokenKind::eFloat) {
		take();
		return SomeVal(literal(token.text, token));
	}

	if (token.kind == TokenKind::eIdentifier) {
		take();
		if (at(TokenKind::eLParen)) {
			return call(token);
		}

		if (at(TokenKind::eLBracket) && isTypeName(token.text)) {
			return arrayConstruct(token);
		}

		return SomeVal(name(token));
	}

	if (accept(TokenKind::eLParen)) {
		const auto inner = expression();
		if (not inner.has_value() || not expect(TokenKind::eRParen, "')'")) {
			return None;
		}
		return SomeVal(*inner);
	}

	syntaxError("an expression");
	return None;
}

/* `T[N](args)` / `T[](args)` at '[': a construct typed as the array; `[]` takes the argument count, `[N]` is checked by typecheck */
Opt<ir::ExprRef> Parser::arrayConstruct(ref<const ir::Token> name_) {
	const auto& open = take();
	u32 count = 0uL;
	if (at(TokenKind::eInteger)) {
		const auto text = take().text;
		for (const char digit : text) {
			count = count * 10uL + static_cast<u32>(digit - '0');
		}

		if (text.size() > 9uz || count == 0uL) {
			error(codes::asl("invalid-array-size"), open, "invalid array size '" + String(text) + "'");
		}
	}

	if (not expect(TokenKind::eRBracket, "']'")) {
		return None;
	}

	if (not at(TokenKind::eLParen)) {
		syntaxError("'(' after the array type");
		return None;
	}

	const auto made = call(name_);
	if (not made.has_value()) {
		return None;
	}

	auto& construct = _shared.mod.exprs[made->index];
	ir::TypeDesc desc {};
	desc.kind = ir::TypeKind::eArray;
	desc.element = construct.type;
	desc.count = count != 0uL ? count : static_cast<u32>(construct.operands.size());
	construct.type = intern(std::move(desc));
	return made;
}

/* `name(args)` at '(' */
Opt<ir::ExprRef> Parser::call(ref<const ir::Token> name_) {
	take();
	Vector<ir::ExprRef> args {};
	if (not accept(TokenKind::eRParen)) {
		do {
			const auto arg = expression();
			if (not arg.has_value()) {
				return None;
			}
			args.emplace_back(*arg);
		} while (accept(TokenKind::eComma));

		if (not expect(TokenKind::eRParen, "',' or ')'")) {
			return None;
		}
	}

	return SomeVal(callee(name_, std::move(args)));
}

/* type name -> construct, builtin function -> builtin, else a call resolved after the parse */
ir::ExprRef Parser::callee(ref<const ir::Token> name_, Vector<ir::ExprRef> args_) {
	if (const auto type = namedType(name_.text); type.has_value()) {
		ir::Expr construct {};
		construct.kind = ir::ExprKind::eConstruct;
		construct.name = name_.text;
		construct.type = *type;
		construct.operands = std::move(args_);
		construct.loc = name_.loc;
		return expr(std::move(construct));
	}

	if (name_.text == "len" || ir::findBuiltinFunction(name_.text) != nullptr) {
		return node(ir::ExprKind::eBuiltin, name_.text, std::move(args_), name_.loc);
	}

	const auto pending = node(ir::ExprKind::eCall, name_.text, std::move(args_), name_.loc);
	_shared.calls.emplace_back(
		PendingCall { diagnostic(codes::asl("unknown-function"), name_, "unknown function '" + String(name_.text) + "'"), pending.index }
	);
	return pending;
}

/* `true`/`false` -> literal; else local, then binding (by local name), then builtin variable */
ir::ExprRef Parser::name(ref<const ir::Token> name_) {
	const auto text = name_.text;
	if (text == "true" || text == "false") {
		return literal(text, name_);
	}

	ir::Expr named {};
	named.kind = ir::ExprKind::eNameRef;
	named.name = text;
	named.loc = name_.loc;
	const auto& bindings = _shared.mod.bindings;
	const auto binding = std::ranges::find(bindings, text, &ir::BindingDecl::localName);
	const auto& constants = _shared.mod.constants;
	const auto constant = std::ranges::find(constants, text, &ir::ConstantDecl::name);

	if (std::ranges::find(_locals, text) != _locals.end()) {
		named.local = true;

	} else if (constant != constants.end()) {
		named.constant = ir::ConstantRef { static_cast<u32>(constant - constants.begin()) };

	} else if (binding != bindings.end()) {
		named.binding = ir::BindingRef { static_cast<u32>(binding - bindings.begin()) };

	} else if (isBuiltinVariable(text)) {
		named.kind = ir::ExprKind::eBuiltin;

	} else {
		error(codes::asl("unknown-name"), name_, "unknown name '" + String(text) + "'");
	}

	// a local is never a binding use (usageOf relies on it)
	assert(not named.local || named.binding.index == ir::BindingRef {}.index);
	return expr(std::move(named));
}
