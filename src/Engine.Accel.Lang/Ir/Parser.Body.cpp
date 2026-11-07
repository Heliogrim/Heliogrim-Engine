#include "ParserImpl.hpp"

#include <array>
#include <cassert>

#include "Builtin.hpp"
#include "Typecheck.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;
using namespace hg::engine::accel::lang::ir::detail;

namespace {
	using ir::findBuiltinFunction;

	[[nodiscard]] bool isBuiltinVariable(StringView name_) {
		return ir::findBuiltinVariable(name_) != nullptr;
	}


	/* statement keywords the lexer leaves as identifiers; loops and switch come after this milestone */
	constexpr std::array<StringView, 3> unsupportedStatements { "for", "while", "switch" };

	constexpr u32 additive = 2uL;

	/* binary operator precedence, higher binds tighter (all left-associative); 0 = no binary operator */
	[[nodiscard]] u32 precedenceOf(TokenKind kind_) {
		if (kind_ == TokenKind::eStar || kind_ == TokenKind::eSlash) {
			return additive + 1uL;
		}
		if (kind_ == TokenKind::ePlus || kind_ == TokenKind::eMinus) {
			return additive;
		}
		const bool comparison = kind_ == TokenKind::eEqEq || kind_ == TokenKind::eNotEq || kind_ == TokenKind::eLess ||
			kind_ == TokenKind::eLessEq || kind_ == TokenKind::eGreater || kind_ == TokenKind::eGreaterEq;
		return comparison ? 1uL : 0uL;
	}
}

/*
 * Bodies. Every statement recursion passes through statement(), every expression recursion through unary(),
 * both under the shared Nest budget. Recursive frames stay small: node building and diagnostics live in
 * non-recursive helpers. A parser recovers only from errors in its own tokens (fail()); a failed
 * sub-statement has recovered already and is passed up as None.
 */

ir::ExprRef Parser::expr(ir::Expr expr_) {
	auto& exprs = _shared.module.exprs;
	exprs.emplace_back(std::move(expr_));
	return ir::ExprRef { static_cast<u32>(exprs.size() - 1uz) };
}

ir::ExprRef Parser::node(ir::ExprKind kind_, StringView name_, Vector<ir::ExprRef> operands_, ir::SourceLoc loc_) {
	ir::Expr out {};
	out.kind = kind_;
	out.name = name_;
	out.operands = std::move(operands_);
	out.loc = loc_;
	return expr(std::move(out));
}

ir::StmtRef Parser::add(ir::Stmt stmt_) {
	auto& stmts = _shared.module.stmts;
	const auto &exprs = _shared.module.exprs;
	if (stmt_.lhs.index < exprs.size()) {
		stmt_.loc = exprs[stmt_.lhs.index].loc;
	} else if (_pos > 0uz) {
		stmt_.loc = _tokens[_pos - 1uz].loc;
	}
	stmts.emplace_back(std::move(stmt_));
	return ir::StmtRef { static_cast<u32>(stmts.size() - 1uz) };
}

ir::StmtRef Parser::stmt(ir::StmtKind kind_, ir::ExprRef lhs_, Vector<ir::StmtRef> children_) {
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
		error(open_, "Unterminated block, expected '}'.");
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
		return block();
	}
	if (accept(TokenKind::eSemicolon)) {
		return stmt(ir::StmtKind::eBlock);
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
	if (head.kind == TokenKind::eIdentifier) {
		if (std::ranges::find(unsupportedStatements, head.text) != unsupportedStatements.end()) {
			take();
			return unsupported(head, head.text);
		}
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
	_shared.module.stmts[head->index].children.emplace_back(*child);
	return *head;
}

/* keeps the condition out of the recursive frame */
Opt<ir::StmtRef> Parser::whenHead() {
	take();
	if (not accept(TokenKind::eKwWhen)) {
		error(peek(), "Only @when applies to a statement.");
		return None;
	}
	auto cond = parenCondition();
	if (not cond.has_value()) {
		return None;
	}
	ir::Stmt when {};
	when.kind = ir::StmtKind::eWhen;
	when.when = std::move(*cond);
	return add(std::move(when));
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
		return stmt(ir::StmtKind::eIf, *cond, { *then });
	}
	const auto otherwise = branch();
	if (not otherwise.has_value()) {
		return None;
	}
	return stmt(ir::StmtKind::eIf, *cond, { *then, *otherwise });
}

Opt<ir::StmtRef> Parser::returnStatement() {
	take();
	if (accept(TokenKind::eSemicolon)) {
		return stmt(ir::StmtKind::eReturn);
	}
	const auto value = expression();
	if (not value.has_value() || not expect(TokenKind::eSemicolon, "';'")) {
		return fail();
	}
	return stmt(ir::StmtKind::eReturn, *value);
}

/* `let|const name [: Type] [= init|&place|&mut place];`, a type or an initializer required */
Opt<ir::StmtRef> Parser::letStatement(bool const_) {
	take();
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
		error(name, "A '" + String(const_ ? "const" : "let") + "' declaration needs a type or an initializer.");
		return fail();
	}
	return local(name, declType.value_or(ir::TypeRef {}), const_);
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
Opt<ir::StmtRef> Parser::local(ref<const ir::Token> name_, ir::TypeRef type_, bool const_) {
	ir::Stmt decl {};
	decl.kind = ir::StmtKind::eDeclareLocal;
	decl.name = name_.text;
	decl.type = type_;
	decl.mutable_ = not const_;
	bool valid = true;
	if (name_.text.starts_with("_hg_")) {
		error(name_, "Identifiers starting with '_hg_' are reserved.");
	}
	if (accept(TokenKind::eAssign)) {
		if (accept(TokenKind::eAmp)) {
			decl.kind = ir::StmtKind::eDeclareRef;
			decl.mutable_ = accept(TokenKind::eKwMut);
			if (decl.mutable_ && const_) {
				error(name_, "cannot take '&mut' into a 'const' alias");
			}
			if (type_.index != ir::TypeRef {}.index) {
				error(name_, "A reference declaration cannot have a type annotation.");
			}
		}
		const auto init = expression();
		valid = init.has_value();
		decl.lhs = valid ? *init : decl.lhs;
	}
	valid = valid && expect(TokenKind::eSemicolon, "';'");

	if (std::find(_locals.begin() + static_cast<ptrdiff_t>(_scope), _locals.end(), name_.text) != _locals.end()) {
		error(name_, "Duplicate local '" + String(name_.text) + "'.");
	}
	_locals.emplace_back(name_.text);
	if (not valid) {
		return fail();
	}
	return add(std::move(decl));
}

/* `expr;` or `lvalue = expr;` */
Opt<ir::StmtRef> Parser::simpleStatement() {
	const auto& start = peek();
	const auto lhs = expression();
	if (not lhs.has_value()) {
		return fail();
	}
	if (compoundAhead()) {
		return unsupported(peek(), String(peek().text) + String(next().text));
	}
	if (not accept(TokenKind::eAssign)) {
		if (not expect(TokenKind::eSemicolon, "';'")) {
			return fail();
		}
		return stmt(ir::StmtKind::eExpr, *lhs);
	}
	if (not assignable(*lhs)) {
		error(start, "Cannot assign to this expression.");
	}
	const auto rhs = expression();
	if (not rhs.has_value() || not expect(TokenKind::eSemicolon, "';'")) {
		return fail();
	}
	ir::Stmt assign {};
	assign.kind = ir::StmtKind::eAssign;
	assign.lhs = *lhs;
	assign.rhs = *rhs;
	return add(std::move(assign));
}

/* reports at `at_`, then skips `(...)` and a `{...}` body or the rest of the statement */
Opt<ir::StmtRef> Parser::unsupported(ref<const ir::Token> at_, StringView what_) {
	error(at_, "'" + String(what_) + "' is unsupported in this milestone.");
	if (at(TokenKind::eLParen)) {
		skipGroup(TokenKind::eLParen, TokenKind::eRParen);
	}
	if (at(TokenKind::eLBrace)) {
		skipGroup(TokenKind::eLBrace, TokenKind::eRBrace);
		return None;
	}
	return fail();
}

/* `+=`, `-=`, `*=`, `/=`, `++` (lexed as two tokens) */
bool Parser::compoundAhead() const {
	const auto kind = peek().kind;
	const auto ahead = next().kind;
	return precedenceOf(kind) >= additive && (ahead == TokenKind::eAssign || (kind == TokenKind::ePlus && ahead == kind));
}

/* a name, optionally behind field and index accesses */
bool Parser::assignable(ir::ExprRef ref_) const {
	return ir::isPlace(_shared.module, ref_);
}

/* Expressions: comparisons < `+ -` < `* /` (precedence climbing) < unary `- !` < postfix `.name [i]` < primary */

Opt<ir::ExprRef> Parser::expression(u32 minPrecedence_) {
	auto lhs = unary();
	while (lhs.has_value() && precedenceOf(peek().kind) >= minPrecedence_ && not compoundAhead()) {
		const auto& op = take();
		const auto rhs = expression(precedenceOf(op.kind) + 1uL);
		lhs = rhs.has_value() ? Opt<ir::ExprRef> { node(ir::ExprKind::eBinary, op.text, { *lhs, *rhs }, op.loc) } : None;
	}
	return lhs;
}

Opt<ir::ExprRef> Parser::unary() {
	const Nest nest { _shared.depth };
	if (tooDeep(peek())) {
		return None;
	}
	if (at(TokenKind::eMinus) || at(TokenKind::eBang)) {
		const auto& op = take();
		const auto operand = unary();
		if (not operand.has_value()) {
			return None;
		}
		return node(ir::ExprKind::eUnary, op.text, { *operand }, op.loc);
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
			base_ = node(ir::ExprKind::eField, take().text, { *base_ }, _shared.module.exprs[base_->index].loc);
		} else if (accept(TokenKind::eLBracket)) {
			const auto index = expression();
			if (not index.has_value() || not expect(TokenKind::eRBracket, "']'")) {
				return None;
			}
			base_ = node(ir::ExprKind::eIndex, {}, { *base_, *index }, _shared.module.exprs[base_->index].loc);
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
		return node(ir::ExprKind::eLiteral, token.text, {}, token.loc);
	}
	if (token.kind == TokenKind::eIdentifier) {
		take();
		if (at(TokenKind::eLParen)) {
			return call(token);
		}
		return name(token);
	}
	if (accept(TokenKind::eLParen)) {
		const auto inner = expression();
		if (not inner.has_value() || not expect(TokenKind::eRParen, "')'")) {
			return None;
		}
		return *inner;
	}
	syntaxError("an expression");
	return None;
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
	return callee(name_, std::move(args));
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
	if (ir::findBuiltinFunction(name_.text) != nullptr) {
		return node(ir::ExprKind::eBuiltin, name_.text, std::move(args_), name_.loc);
	}
	const auto pending = node(ir::ExprKind::eCall, name_.text, std::move(args_), name_.loc);
	_shared.calls.emplace_back(PendingCall { diagnostic(name_, "Unknown function '" + String(name_.text) + "'."), pending.index });
	return pending;
}

/* `true`/`false` -> literal; else local, then binding (by local name), then builtin variable */
ir::ExprRef Parser::name(ref<const ir::Token> name_) {
	const auto text = name_.text;
	if (text == "true" || text == "false") {
		return node(ir::ExprKind::eLiteral, text, {}, name_.loc);
	}

	ir::Expr named {};
	named.kind = ir::ExprKind::eNameRef;
	named.name = text;
	named.loc = name_.loc;
	const auto& bindings = _shared.module.bindings;
	const auto binding = std::ranges::find(bindings, text, &ir::BindingDecl::localName);
	if (std::ranges::find(_locals, text) != _locals.end()) {
		named.local = true;
	} else if (binding != bindings.end()) {
		named.binding = ir::BindingRef { static_cast<u32>(binding - bindings.begin()) };
	} else if (isBuiltinVariable(text)) {
		named.kind = ir::ExprKind::eBuiltin;
	} else {
		error(name_, "Unknown name '" + String(text) + "'.");
	}
	// a local is never a binding use (usageOf relies on it)
	assert(not named.local || named.binding.index == ir::BindingRef {}.index);
	return expr(std::move(named));
}
