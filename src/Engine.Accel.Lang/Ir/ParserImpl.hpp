#pragma once

/*
 * Internal to the `.asl` parser: the grammar is one class, defined across Parser.cpp (module level: includes,
 * annotations, conditions, types, declarations) and Parser.Body.cpp (function bodies: statements and expressions).
 */

#include <algorithm>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Lexer.hpp"
#include "Parser.hpp"

namespace hg::engine::accel::lang::ir::detail {
	using ir::TokenKind;

	/* RAII lexical scope over the locals in view: names declared inside end with it */
	class Scope {
	public:
		Scope(ref<Vector<StringView>> locals_, ref<size_t> start_) :
			_locals(locals_),
			_start(start_),
			_outer(start_) {
			_start = _locals.size();
		}

		Scope(ref<const Scope>) = delete;
		ref<Scope> operator=(ref<const Scope>) = delete;

		~Scope() {
			_locals.resize(_start);
			_start = _outer;
		}

	private:
		ref<Vector<StringView>> _locals;
		ref<size_t> _start;
		size_t _outer;
	};

	/* a call resolved after the whole parse, so functions may be called before their declaration */
	struct PendingCall {
		ir::Diagnostic unknown;
		size_t expr;
	};

	/* RAII nesting level */
	class Nest {
	public:
		explicit Nest(ref<u32> depth_) :
			_depth(depth_) {
			++_depth;
		}

		Nest(ref<const Nest>) = delete;
		ref<Nest> operator=(ref<const Nest>) = delete;

		~Nest() {
			--_depth;
		}

	private:
		ref<u32> _depth;
	};

	/*
	 * State shared by the root module and all modules it includes; everything is parsed into one Module.
	 * shortcut: modules in one parse share one name scope, so whether a sibling's toggles/structs are accepted depends
	 * on include order; upgrade when per-module scopes are needed (e.g. with Task 6 merge).
	 */
	struct Shared {
		explicit Shared(ref<const ir::ModuleResolver> resolver_) :
			resolver(resolver_) {}

		ref<const ir::ModuleResolver> resolver;
		ir::Module module {};
		ir::Diagnostics diags {};
		/* include chain, root first */
		Vector<String> active {};
		Vector<String> done {};
		Vector<PendingCall> calls {};
		/* nesting across includes, conditions, @when blocks, statements and expressions: one stack budget per parse */
		u32 depth = 0uL;
		/* after a depth error, unterminated enclosing blocks are not reported again */
		u32 depthErrors = 0uL;
	};

	/* What precedes a declaration: annotations, `implicit` and the enclosing condition */
	struct Annotations {
		/* id, mode, bindClass, implicit, input and when as collected; type and localName unused */
		ir::BindingDecl binding {};
		/* the @input / @output / @redefine names, for diagnostics */
		const ir::Token* direction = nullptr;
		const ir::Token* redefine = nullptr;
		/* the `true` of `@redefine (allow = true)`, if given */
		const ir::Token* allow = nullptr;
		/* the @spec name; its id is collected in `binding.id` */
		const ir::Token* spec = nullptr;
	};

	class Parser {
	public:
		Parser(StringView name_, ref<const Vector<ir::Token>> tokens_, ref<Shared> shared_) :
			_name(name_),
			_tokens(tokens_),
			_shared(shared_) {}

		void run();

	private:
		StringView _name;
		ref<const Vector<ir::Token>> _tokens;
		ref<Shared> _shared;
		size_t _pos = 0uz;
		/* locals in view inside a function body (parameters first); `_scope` starts the innermost scope */
		Vector<StringView> _locals {};
		size_t _scope = 0uz;
		/* errors at end of input; an unterminated block is then not reported again */
		size_t _eofErrors = 0uz;

		/* call after entering a Nest on `_shared.depth`; reports at the offending token */
		[[nodiscard]] bool tooDeep(ref<const ir::Token> at_);

		/* the root module's diagnostics are unprefixed */
		[[nodiscard]] bool nested() const {
			return _shared.active.size() > 1uz;
		}

		/* Token access; the trailing eEof is never stepped over. */

		[[nodiscard]] ref<const ir::Token> peek() const {
			return _tokens[_pos];
		}

		[[nodiscard]] bool at(TokenKind kind_) const {
			return peek().kind == kind_;
		}

		ref<const ir::Token> take() {
			const auto& token = _tokens[_pos];
			if (_pos + 1uz < _tokens.size()) {
				++_pos;
			}
			return token;
		}

		bool accept(TokenKind kind_) {
			if (not at(kind_)) {
				return false;
			}
			take();
			return true;
		}

		[[nodiscard]] ref<const ir::Token> next() const {
			return _tokens[std::min(_pos + 1uz, _tokens.size() - 1uz)];
		}

		[[nodiscard]] ir::Diagnostic diagnostic(ref<const ir::Token> at_, String message_) const;
		void error(ref<const ir::Token> at_, String message_);
		void syntaxError(StringView expected_);
		bool expect(TokenKind kind_, StringView expected_);

		/* Error recovery: skip past the next `;` or balanced `{...}`, or stop before an unmatched `}`. */
		void recover();
		/* At `open_`: skips to after the matching `close_` (or to the end of input). */
		void skipGroup(TokenKind open_, TokenKind close_);

		/* Items */

		void item(ir::Condition when_);
		void include();
		void declaration(ir::Condition when_);
		void whenBlock(ref<const ir::Condition> when_);
		bool annotationArgs(ref<const ir::Token> annotation_, ref<Annotations> ann_);
		void annotationValue(ref<const ir::Token> annotation_, ref<const ir::Token> key_, ref<const ir::Token> value_, ref<Annotations> ann_);

		/* Conditions */

		[[nodiscard]] Opt<ir::Condition> parenCondition();
		[[nodiscard]] Opt<ir::Condition> condition();
		[[nodiscard]] Opt<ir::Condition> chain(TokenKind op_, bool conjunction_);
		[[nodiscard]] Opt<ir::Condition> unaryCondition();
		[[nodiscard]] Opt<ir::Condition> combinator(ref<const ir::Token> name_);
		void unknownCombinator(ref<const ir::Token> name_);
		[[nodiscard]] Opt<ir::Condition> comparison(ref<const ir::Token> name);

		/* Types */

		[[nodiscard]] ir::TypeRef intern(ir::TypeDesc desc_);
		[[nodiscard]] Opt<ir::TypeRef> findStruct(StringView name_) const;
		[[nodiscard]] bool isTypeName(StringView name_) const;
		[[nodiscard]] Opt<ir::TypeRef> namedType(StringView name_);
		[[nodiscard]] Opt<ir::TypeRef> type();
		[[nodiscard]] Opt<Vector<ir::FieldDesc>> fields();

		/* Declarations */

		void toggleDecl();
		void structDecl();
		void function(ref<const Annotations> ann_);
		void binding(ir::BindingDecl decl_);
		void constantDecl(ref<const Annotations> ann_);

		/* Bodies (Parser.Body.cpp) */

		[[nodiscard]] ir::ExprRef expr(ir::Expr expr_);
		[[nodiscard]] ir::ExprRef node(ir::ExprKind kind_, StringView name_, Vector<ir::ExprRef> operands_ = {}, ir::SourceLoc loc_ = {});
		[[nodiscard]] ir::StmtRef add(ir::Stmt stmt_);
		[[nodiscard]] ir::StmtRef stmt(ir::StmtKind kind_, ir::ExprRef lhs_ = {}, Vector<ir::StmtRef> children_ = {});
		[[nodiscard]] Opt<ir::StmtRef> fail();
		void unterminated(ref<const ir::Token> open_);

		[[nodiscard]] ir::StmtRef block();
		[[nodiscard]] Opt<ir::StmtRef> statement();
		[[nodiscard]] Opt<ir::StmtRef> branch();
		[[nodiscard]] Opt<ir::StmtRef> whenStatement();
		[[nodiscard]] Opt<ir::StmtRef> whenHead();
		[[nodiscard]] Opt<ir::StmtRef> ifStatement();
		[[nodiscard]] Opt<ir::StmtRef> returnStatement();
		[[nodiscard]] Opt<ir::StmtRef> letStatement(bool const_);
		[[nodiscard]] Opt<ir::StmtRef> typedDeclaration();
		[[nodiscard]] Opt<ir::StmtRef> local(ref<const ir::Token> name_, ir::TypeRef type_, bool const_);
		[[nodiscard]] Opt<ir::StmtRef> whileStatement();
		[[nodiscard]] Opt<ir::StmtRef> forStatement();
		[[nodiscard]] Opt<ir::StmtRef> simpleStatement(bool terminated_ = true);
		[[nodiscard]] bool assignable(ir::ExprRef ref_) const;

		[[nodiscard]] Opt<ir::ExprRef> expression(u32 minPrecedence_ = 1uL);
		[[nodiscard]] Opt<ir::ExprRef> ternary(ir::ExprRef cond_);
		[[nodiscard]] Opt<ir::ExprRef> unary();
		[[nodiscard]] Opt<ir::ExprRef> postfix(Opt<ir::ExprRef> base_);
		[[nodiscard]] Opt<ir::ExprRef> primary();
		[[nodiscard]] Opt<ir::ExprRef> arrayConstruct(ref<const ir::Token> name_);
		[[nodiscard]] Opt<ir::ExprRef> call(ref<const ir::Token> name_);
		[[nodiscard]] ir::ExprRef callee(ref<const ir::Token> name_, Vector<ir::ExprRef> args_);
		[[nodiscard]] ir::ExprRef name(ref<const ir::Token> name_);
	};
}
