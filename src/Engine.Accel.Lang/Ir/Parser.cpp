#include "Parser.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <string>

#include "Lexer.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	using ir::TokenKind;

	struct BuiltinType {
		StringView name;
		ir::TypeKind kind;
		ir::ScalarKind scalar;
		u16 bits;
		u16 n;
		u16 m;
	};

	constexpr std::array<BuiltinType, 12> builtinTypes {
		{
			{ "bool", ir::TypeKind::eScalar, ir::ScalarKind::eBool, 32u, 1u, 1u },
			{ "f32", ir::TypeKind::eScalar, ir::ScalarKind::eFloat, 32u, 1u, 1u },
			{ "f32vec2", ir::TypeKind::eVector, ir::ScalarKind::eFloat, 32u, 2u, 1u },
			{ "f32vec3", ir::TypeKind::eVector, ir::ScalarKind::eFloat, 32u, 3u, 1u },
			{ "f32vec4", ir::TypeKind::eVector, ir::ScalarKind::eFloat, 32u, 4u, 1u },
			{ "f32mat4", ir::TypeKind::eMatrix, ir::ScalarKind::eFloat, 32u, 4u, 4u },
			{ "u32", ir::TypeKind::eScalar, ir::ScalarKind::eUInt, 32u, 1u, 1u },
			{ "u8", ir::TypeKind::eScalar, ir::ScalarKind::eUInt, 8u, 1u, 1u },
			{ "u8vec4", ir::TypeKind::eVector, ir::ScalarKind::eUInt, 8u, 4u, 1u },
			{ "s32", ir::TypeKind::eScalar, ir::ScalarKind::eInt, 32u, 1u, 1u },
			{ "texture2d", ir::TypeKind::eTexture, ir::ScalarKind::eFloat, 32u, 1u, 1u },
			{ "sampler", ir::TypeKind::eSampler, ir::ScalarKind::eFloat, 32u, 1u, 1u }
		}
	};

	[[nodiscard]] const BuiltinType* findBuiltin(StringView name_) {
		const auto it = std::ranges::find(builtinTypes, name_, &BuiltinType::name);
		return it != builtinTypes.end() ? &*it : nullptr;
	}

	/* Builtin names (expression kind eBuiltin); Task 9 maps each to its GLSL form; extended per shader in Tasks 12-14 */
	constexpr std::array<StringView, 4> builtinVariables {
		"vertex_index",// s32, gl_VertexIndex
		"instance_index",// s32, gl_InstanceIndex
		"position",// f32vec4, gl_Position
		"frag_depth"// f32, gl_FragDepth
	};

	constexpr std::array<StringView, 1> builtinFunctions {
		"sample"// sample(texture2d, sampler, f32vec2) -> f32vec4, texture(sampler2D(tex, smp), uv)
	};

	template <size_t Size_>
	[[nodiscard]] bool contains(cref<std::array<StringView, Size_>> names_, StringView name_) {
		return std::ranges::find(names_, name_) != names_.end();
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

	/* RAII lexical scope over the locals in view: names declared inside end with it */
	class Scope {
	public:
		Scope(ref<Vector<StringView>> locals_, ref<size_t> start_) :
			_locals(locals_),
			_start(start_),
			_outer(start_) {
			_start = _locals.size();
		}

		Scope(cref<Scope>) = delete;
		ref<Scope> operator=(cref<Scope>) = delete;

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

	[[nodiscard]] Opt<ir::BindMode> bindModeOf(StringView text_) {
		if (text_ == "forward") {
			return ir::BindMode::eForward;
		}
		if (text_ == "bind") {
			return ir::BindMode::eBind;
		}
		return None;
	}

	[[nodiscard]] Opt<ir::BindClass> bindClassOf(StringView text_) {
		if (text_ == "uniform") {
			return ir::BindClass::eUniformBuffer;
		}
		if (text_ == "storage") {
			return ir::BindClass::eStorageBuffer;
		}
		if (text_ == "texture") {
			return ir::BindClass::eTexture;
		}
		if (text_ == "sampler") {
			return ir::BindClass::eSampler;
		}
		if (text_ == "push") {
			return ir::BindClass::ePushConstant;
		}
		return None;
	}

	[[nodiscard]] String describe(cref<ir::Token> token_) {
		return token_.kind == TokenKind::eEof ? String("end of input") : "'" + String(token_.text) + "'";
	}

	[[nodiscard]] ir::Condition conjoin(ir::Condition outer_, ir::Condition inner_) {
		if (outer_.kind == ir::ConditionKind::eTrue) {
			return inner_;
		}
		return ir::Condition::all({ std::move(outer_), std::move(inner_) });
	}

	/* One limit for every recursive production (conditions, @when blocks, includes, statements, expressions):
	 * bad input must not overflow the stack */
	constexpr u32 maxDepth = 256uL;

	/* RAII nesting level */
	class Nest {
	public:
		explicit Nest(ref<u32> depth_) :
			_depth(depth_) {
			++_depth;
		}

		Nest(cref<Nest>) = delete;
		ref<Nest> operator=(cref<Nest>) = delete;

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
		explicit Shared(cref<ir::ModuleResolver> resolver_) :
			resolver(resolver_) {}

		cref<ir::ModuleResolver> resolver;
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

	void parseSource(StringView name_, StringView source_, ref<Shared> shared_);

	[[nodiscard]] String located(StringView module_, bool nested_, String message_) {
		return nested_ ? "in '" + String(module_) + "': " + message_ : message_;
	}

	/* What precedes a declaration: annotations, `implicit` and the enclosing condition */
	struct Annotations {
		/* id, mode, bindClass, implicit, input and when as collected; type and localName unused */
		ir::BindingDecl binding {};
		/* the @input / @output / @redefine names, for diagnostics */
		const ir::Token* direction = nullptr;
		const ir::Token* redefine = nullptr;
		/* the `true` of `@redefine (allow = true)`, if given */
		const ir::Token* allow = nullptr;
	};

	class Parser {
	public:
		Parser(StringView name_, cref<Vector<ir::Token>> tokens_, ref<Shared> shared_) :
			_name(name_),
			_tokens(tokens_),
			_shared(shared_) {}

		void run() {
			while (not at(TokenKind::eEof)) {
				if (at(TokenKind::eRBrace)) {
					error(take(), "Unmatched '}'.");
					continue;
				}
				item(ir::Condition::always());
			}
		}

	private:
		StringView _name;
		cref<Vector<ir::Token>> _tokens;
		ref<Shared> _shared;
		size_t _pos = 0uz;
		/* locals in view inside a function body (parameters first); `_scope` starts the innermost scope */
		Vector<StringView> _locals {};
		size_t _scope = 0uz;
		/* errors at end of input; an unterminated block is then not reported again */
		size_t _eofErrors = 0uz;

		/* call after entering a Nest on `_shared.depth`; reports at the offending token */
		[[nodiscard]] bool tooDeep(cref<ir::Token> at_) {
			if (_shared.depth <= maxDepth) {
				return false;
			}
			++_shared.depthErrors;
			error(at_, "Nesting too deep (limit " + std::to_string(maxDepth) + ").");
			return true;
		}

		/* the root module's diagnostics are unprefixed */
		[[nodiscard]] bool nested() const {
			return _shared.active.size() > 1uz;
		}

		/* Token access; the trailing eEof is never stepped over. */

		[[nodiscard]] cref<ir::Token> peek() const {
			return _tokens[_pos];
		}

		[[nodiscard]] bool at(TokenKind kind_) const {
			return peek().kind == kind_;
		}

		cref<ir::Token> take() {
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

		[[nodiscard]] cref<ir::Token> next() const {
			return _tokens[std::min(_pos + 1uz, _tokens.size() - 1uz)];
		}

		[[nodiscard]] ir::Diagnostic diagnostic(cref<ir::Token> at_, String message_) const {
			return ir::Diagnostic {
				ir::Severity::eError, located(_name, nested(), std::move(message_)), at_.loc.line, at_.loc.column
			};
		}

		void error(cref<ir::Token> at_, String message_) {
			if (at_.kind == TokenKind::eEof) {
				++_eofErrors;
			}
			_shared.diags.push_back(diagnostic(at_, std::move(message_)));
		}

		void syntaxError(StringView expected_) {
			error(peek(), "Expected " + String(expected_) + ", found " + describe(peek()) + ".");
		}

		bool expect(TokenKind kind_, StringView expected_) {
			if (accept(kind_)) {
				return true;
			}
			syntaxError(expected_);
			return false;
		}

		/* Error recovery: skip past the next `;` or balanced `{...}`, or stop before an unmatched `}`. */
		void recover() {
			while (not at(TokenKind::eEof) && not at(TokenKind::eRBrace)) {
				if (accept(TokenKind::eSemicolon)) {
					return;
				}
				if (at(TokenKind::eLBrace)) {
					return skipGroup(TokenKind::eLBrace, TokenKind::eRBrace);
				}
				take();
			}
		}

		/* At `open_`: skips to after the matching `close_` (or to the end of input). */
		void skipGroup(TokenKind open_, TokenKind close_) {
			size_t depth = 0uz;
			do {
				if (at(open_)) {
					++depth;
				} else if (at(close_)) {
					--depth;
				}
				take();
			} while (depth > 0uz && not at(TokenKind::eEof));
		}

		/* Items */

		void item(ir::Condition when_) {
			if (accept(TokenKind::eSemicolon)) {
				return;
			}
			if (at(TokenKind::eInclude)) {
				return include();
			}
			declaration(std::move(when_));
		}

		void include() {
			// allowed only while nothing but includes precede it (so also never inside a @when block)
			const bool allowed = std::all_of(
				_tokens.begin(),
				_tokens.begin() + static_cast<ptrdiff_t>(_pos),
				[](cref<ir::Token> token_) {
					return token_.kind == TokenKind::eInclude || token_.kind == TokenKind::eIncludeTarget ||
						token_.kind == TokenKind::eSemicolon;
				}
			);
			const auto& directive = take();
			if (not at(TokenKind::eIncludeTarget)) {
				return syntaxError("an include target");
			}
			const auto& target = take();
			if (not allowed) {
				return error(directive, "#include must precede all declarations of a module.");
			}

			String name { target.text };
			if (std::ranges::find(_shared.active, name) != _shared.active.end()) {
				String chain;
				for (const auto& link : _shared.active) {
					chain += link + " -> ";
				}
				return error(target, "Include cycle: " + chain + name + ".");
			}
			if (std::ranges::find(_shared.done, name) != _shared.done.end()) {
				return;
			}
			const Nest nest { _shared.depth };
			if (tooDeep(target)) {
				return;
			}

			Opt<String> source = None;
			if (_shared.resolver) {
				source = _shared.resolver(name);
			}
			if (not source.has_value()) {
				return error(target, "Unknown include '" + name + "'.");
			}
			// the nested tokens view `*source`; it outlives them since the nested parse completes here
			parseSource(name, *source, _shared);
		}

		void declaration(ir::Condition when_) {
			Annotations ann {};
			ann.binding.when = std::move(when_);

			while (at(TokenKind::eAt)) {
				take();
				if (accept(TokenKind::eKwWhen)) {
					auto cond = parenCondition();
					if (not cond.has_value()) {
						return recover();
					}
					ann.binding.when = conjoin(std::move(ann.binding.when), std::move(*cond));
					if (at(TokenKind::eLBrace)) {
						if (ann.direction != nullptr || ann.redefine != nullptr) {
							error(peek(), "Only @when may precede a '{' block.");
						}
						return whenBlock(ann.binding.when);
					}
					continue;
				}

				if (not at(TokenKind::eIdentifier)) {
					syntaxError("an annotation name");
					return recover();
				}
				const auto& name = take();
				if (name.text == "input" || name.text == "output") {
					if (ann.direction != nullptr) {
						error(name, "Duplicate @input/@output annotation.");
					}
					ann.direction = &name;
					ann.binding.input = name.text == "input";
				} else if (name.text == "redefine") {
					ann.redefine = &name;
				} else {
					error(name, "Unknown annotation '@" + String(name.text) + "'.");
					if (at(TokenKind::eLParen)) {
						skipGroup(TokenKind::eLParen, TokenKind::eRParen);
					}
					continue;
				}
				if (at(TokenKind::eLParen) && not annotationArgs(name, ann)) {
					return recover();
				}
			}

			ann.binding.implicit = accept(TokenKind::eKwImplicit);

			const auto& head = peek();
			if (head.kind == TokenKind::eKwToggle || head.kind == TokenKind::eKwStruct) {
				if (ann.direction != nullptr || ann.redefine != nullptr || ann.binding.implicit) {
					error(head, "Annotations do not apply to a " + String(head.text) + " declaration.");
				}
				if (ann.binding.when.kind != ir::ConditionKind::eTrue) {
					error(head, "A " + String(head.text) + " declaration cannot be conditional.");
				}
				return head.kind == TokenKind::eKwToggle ? toggleDecl() : structDecl();
			}
			if (head.kind == TokenKind::eKwFn) {
				if (ann.direction != nullptr || ann.binding.implicit) {
					error(head, "@input, @output and implicit apply to bindings only.");
				}
				return function(ann);
			}
			if (head.kind == TokenKind::eKwBlock || head.kind == TokenKind::eIdentifier) {
				if (ann.direction == nullptr) {
					error(head, "Expected a declaration, found " + describe(head) + " (variables need @input or @output).");
					return recover();
				}
				if (ann.redefine != nullptr) {
					error(*ann.redefine, "@redefine applies to functions only.");
				}
				return binding(std::move(ann.binding));
			}
			syntaxError("a declaration");
			recover();
		}

		void whenBlock(cref<ir::Condition> when_) {
			const Nest nest { _shared.depth };
			if (tooDeep(peek())) {
				return skipGroup(TokenKind::eLBrace, TokenKind::eRBrace);
			}
			const auto& open = take();
			while (not at(TokenKind::eRBrace) && not at(TokenKind::eEof)) {
				item(when_);
			}
			if (not accept(TokenKind::eRBrace) && _shared.depthErrors == 0uL) {
				error(open, "Unterminated @when block, expected '}'.");
			}
		}

		/* `( key = value, ... )` of @input / @output / @redefine; false after a syntax error */
		bool annotationArgs(cref<ir::Token> annotation_, ref<Annotations> ann_) {
			take();
			if (accept(TokenKind::eRParen)) {
				return true;
			}
			do {
				if (not at(TokenKind::eIdentifier)) {
					syntaxError("an annotation key");
					return false;
				}
				const auto& key = take();
				if (not expect(TokenKind::eAssign, "'='")) {
					return false;
				}
				if (not at(TokenKind::eIdentifier) && not at(TokenKind::eString)) {
					syntaxError("an annotation value");
					return false;
				}
				annotationValue(annotation_, key, take(), ann_);
			} while (accept(TokenKind::eComma));
			return expect(TokenKind::eRParen, "')'");
		}

		void annotationValue(cref<ir::Token> annotation_, cref<ir::Token> key_, cref<ir::Token> value_, ref<Annotations> ann_) {
			const bool redefine = annotation_.text == "redefine";
			const bool word = value_.kind == TokenKind::eIdentifier;
			auto& decl = ann_.binding;
			bool valid = true;

			if (redefine && key_.text == "allow") {
				valid = word && (value_.text == "true" || value_.text == "false");
				ann_.allow = valid && value_.text == "true" ? &value_ : nullptr;
			} else if (not redefine && key_.text == "mode") {
				const auto mode = bindModeOf(value_.text);
				valid = word && mode.has_value();
				decl.mode = valid ? *mode : decl.mode;
			} else if (not redefine && key_.text == "class") {
				const auto cls = bindClassOf(value_.text);
				valid = word && cls.has_value();
				decl.bindClass = valid ? *cls : decl.bindClass;
			} else if (not redefine && key_.text == "id") {
				valid = not word && not value_.text.empty();
				decl.id = valid ? String(value_.text) : decl.id;
			} else {
				return error(
					key_,
					"Unknown annotation key '" + String(key_.text) + "' for @" + String(annotation_.text) + "."
				);
			}

			if (not valid) {
				error(value_, "Invalid value '" + String(value_.text) + "' for annotation key '" + String(key_.text) + "'.");
			}
		}

		/* Conditions: `||` < `&&` < `!` < `( )`, `any/all/none( ... )`, `Toggle == Value`, `Toggle != Value` */

		[[nodiscard]] Opt<ir::Condition> parenCondition() {
			if (not expect(TokenKind::eLParen, "'(' after @when")) {
				return None;
			}
			auto cond = condition();
			if (not cond.has_value() || not expect(TokenKind::eRParen, "')'")) {
				return None;
			}
			return std::move(*cond);
		}

		[[nodiscard]] Opt<ir::Condition> condition() {
			return chain(TokenKind::eOrOr, false);
		}

		/* `a op b op ...`: `||` over `&&` terms (conjunction_ = false), `&&` over unary terms (true) */
		[[nodiscard]] Opt<ir::Condition> chain(TokenKind op_, bool conjunction_) {
			Vector<ir::Condition> terms {};
			do {
				auto term = conjunction_ ? unaryCondition() : chain(TokenKind::eAndAnd, true);
				if (not term.has_value()) {
					return None;
				}
				terms.push_back(std::move(*term));
			} while (accept(op_));

			if (terms.size() == 1uz) {
				return std::move(terms.front());
			}
			return conjunction_ ? ir::Condition::all(std::move(terms)) : ir::Condition::any(std::move(terms));
		}

		/* every condition recursion (`!`, `( )`, any/all/none arguments) passes through here */
		[[nodiscard]] Opt<ir::Condition> unaryCondition() {
			const Nest nest { _shared.depth };
			if (tooDeep(peek())) {
				return None;
			}
			if (accept(TokenKind::eBang)) {
				auto inner = unaryCondition();
				if (not inner.has_value()) {
					return None;
				}
				return ir::Condition::negate(std::move(*inner));
			}
			if (accept(TokenKind::eLParen)) {
				auto inner = condition();
				if (not inner.has_value() || not expect(TokenKind::eRParen, "')'")) {
					return None;
				}
				return std::move(*inner);
			}
			if (not at(TokenKind::eIdentifier)) {
				syntaxError("a toggle condition");
				return None;
			}

			const auto& name = take();
			return at(TokenKind::eLParen) ? combinator(name) : comparison(name);
		}

		/* `name(cond, ...)` at '('; recursive, so it keeps its frame small (Debug stack depth) */
		[[nodiscard]] Opt<ir::Condition> combinator(cref<ir::Token> name_) {
			take();
			Vector<ir::Condition> args {};
			do {
				auto arg = condition();
				if (not arg.has_value()) {
					return None;
				}
				args.push_back(std::move(*arg));
			} while (accept(TokenKind::eComma));
			if (not expect(TokenKind::eRParen, "')'")) {
				return None;
			}

			if (name_.text == "any") {
				return ir::Condition::any(std::move(args));
			}
			if (name_.text == "all") {
				return ir::Condition::all(std::move(args));
			}
			if (name_.text == "none") {
				return ir::Condition::none(std::move(args));
			}
			unknownCombinator(name_);
			return ir::Condition::always();
		}

		void unknownCombinator(cref<ir::Token> name_) {
			error(name_, "Unknown condition '" + String(name_.text) + "', expected any, all or none.");
		}

		/* leaf `Toggle == Value` / `Toggle != Value`; never recurses */
		[[nodiscard]] Opt<ir::Condition> comparison(cref<ir::Token> name) {
			const bool equal = at(TokenKind::eEqEq);
			if (not equal && not at(TokenKind::eNotEq)) {
				syntaxError("'==' or '!=' after a toggle name");
				return None;
			}
			take();
			if (not at(TokenKind::eIdentifier)) {
				syntaxError("a toggle value");
				return None;
			}
			const auto& value = take();

			const auto& toggles = _shared.module.toggles;
			const auto toggle = std::ranges::find(toggles, name.text, &ir::Toggle::name);
			if (toggle == toggles.end()) {
				error(name, "Unknown toggle '" + String(name.text) + "'.");
			} else if (std::ranges::find(toggle->values, value.text) == toggle->values.end()) {
				error(value, "Unknown value '" + String(value.text) + "' for toggle '" + String(name.text) + "'.");
			}

			auto cond = ir::Condition::equals(String(name.text), String(value.text));
			return equal ? cond : ir::Condition::negate(std::move(cond));
		}

		/* Types */

		[[nodiscard]] ir::TypeRef intern(ir::TypeDesc desc_) {
			auto& types = _shared.module.types;
			const auto it = std::ranges::find_if(
				types,
				[&](cref<ir::TypeDesc> type_) {
					return type_.kind != ir::TypeKind::eStruct && type_.kind == desc_.kind && type_.scalar == desc_.scalar &&
						type_.bits == desc_.bits && type_.n == desc_.n && type_.m == desc_.m &&
						type_.element == desc_.element && type_.count == desc_.count && type_.dim == desc_.dim;
				}
			);
			if (it != types.end()) {
				return ir::TypeRef { static_cast<u32>(it - types.begin()) };
			}
			types.push_back(std::move(desc_));
			return ir::TypeRef { static_cast<u32>(types.size() - 1uz) };
		}

		[[nodiscard]] Opt<ir::TypeRef> findStruct(StringView name_) const {
			const auto& types = _shared.module.types;
			const auto it = std::ranges::find_if(
				types,
				[&](cref<ir::TypeDesc> type_) {
					return type_.kind == ir::TypeKind::eStruct && not type_.name.empty() && type_.name == name_;
				}
			);
			if (it == types.end()) {
				return None;
			}
			return ir::TypeRef { static_cast<u32>(it - types.begin()) };
		}

		[[nodiscard]] bool isTypeName(StringView name_) const {
			return findBuiltin(name_) != nullptr || findStruct(name_).has_value();
		}

		/* a struct or builtin type by name, interned */
		[[nodiscard]] Opt<ir::TypeRef> namedType(StringView name_) {
			if (const auto* builtin = findBuiltin(name_); builtin != nullptr) {
				ir::TypeDesc desc {};
				desc.kind = builtin->kind;
				desc.scalar = builtin->scalar;
				desc.bits = builtin->bits;
				desc.n = builtin->n;
				desc.m = builtin->m;
				return intern(std::move(desc));
			}
			return findStruct(name_);
		}

		/* `Name` or `Name[N]` / `Name[]` (repeatable); reports and returns None on failure */
		[[nodiscard]] Opt<ir::TypeRef> type() {
			if (not at(TokenKind::eIdentifier)) {
				syntaxError("a type");
				return None;
			}
			const auto& name = take();

			Opt<ir::TypeRef> result = namedType(name.text);
			if (not result.has_value()) {
				error(name, "Unknown type '" + String(name.text) + "'.");
				return None;
			}

			while (at(TokenKind::eLBracket)) {
				const auto& open = take();
				u32 count = 0uL;
				if (at(TokenKind::eInteger)) {
					// the lexer guarantees digits only; 9 digits always fit u32
					const auto text = take().text;
					for (const char digit : text) {
						count = count * 10uL + static_cast<u32>(digit - '0');
					}
					if (text.size() > 9uz || count == 0uL) {
						error(open, "Invalid array size '" + String(text) + "'.");
					}
				}
				if (not expect(TokenKind::eRBracket, "']'")) {
					return None;
				}
				ir::TypeDesc desc {};
				desc.kind = ir::TypeKind::eArray;
				desc.element = *result;
				desc.count = count;
				result = intern(std::move(desc));
			}
			return *result;
		}

		/* `{ (Type name;)* }`; returns None if the closing brace is missing */
		[[nodiscard]] Opt<Vector<ir::FieldDesc>> fields() {
			if (not expect(TokenKind::eLBrace, "'{'")) {
				return None;
			}
			Vector<ir::FieldDesc> out {};
			while (not at(TokenKind::eRBrace) && not at(TokenKind::eEof)) {
				const auto fieldType = type();
				if (fieldType.has_value() && not at(TokenKind::eIdentifier)) {
					syntaxError("a field name");
				} else if (fieldType.has_value()) {
					String name { take().text };
					if (expect(TokenKind::eSemicolon, "';'")) {
						out.push_back(ir::FieldDesc { std::move(name), *fieldType });
						continue;
					}
				}
				recover();
			}
			if (not expect(TokenKind::eRBrace, "'}'")) {
				return None;
			}
			return out;
		}

		/* Declarations */

		void toggleDecl() {
			take();
			if (not at(TokenKind::eIdentifier)) {
				syntaxError("a toggle name");
				return recover();
			}
			const auto& name = take();
			if (not expect(TokenKind::eColon, "':'") || not expect(TokenKind::eLBrace, "'{'")) {
				return recover();
			}
			Vector<const ir::Token*> values {};
			if (not at(TokenKind::eRBrace)) {
				do {
					if (not at(TokenKind::eIdentifier)) {
						syntaxError("a toggle value");
						return recover();
					}
					values.push_back(&take());
				} while (accept(TokenKind::eComma));
			}
			if (not expect(TokenKind::eRBrace, "'}'")) {
				return recover();
			}
			const ir::Token* defaultTok = nullptr;
			if (accept(TokenKind::eAssign)) {
				if (not at(TokenKind::eIdentifier)) {
					syntaxError("a default toggle value");
					return recover();
				}
				defaultTok = &take();
			}
			if (not expect(TokenKind::eSemicolon, "';'")) {
				return recover();
			}

			const String toggleName { name.text };
			ir::Toggle toggle { toggleName, {}, {} };
			for (const auto* value : values) {
				if (std::ranges::find(toggle.values, value->text) != toggle.values.end()) {
					error(*value, "Duplicate value '" + String(value->text) + "' in toggle '" + toggleName + "'.");
				} else {
					toggle.values.emplace_back(value->text);
				}
			}
			if (toggle.values.empty()) {
				error(name, "Toggle '" + toggleName + "' declares no values.");
			}
			if (defaultTok != nullptr) {
				if (std::ranges::find(toggle.values, defaultTok->text) == toggle.values.end()) {
					error(*defaultTok, "Default value '" + String(defaultTok->text) + "' is not a value of toggle '" + toggleName + "'.");
				}
				toggle.defaultValue = defaultTok->text;
			} else if (not toggle.values.empty()) {
				toggle.defaultValue = toggle.values.front();
			}

			auto& toggles = _shared.module.toggles;
			if (std::ranges::find(toggles, toggleName, &ir::Toggle::name) != toggles.end()) {
				return error(name, "Duplicate toggle '" + toggleName + "'.");
			}
			toggles.push_back(std::move(toggle));
		}

		void structDecl() {
			take();
			if (not at(TokenKind::eIdentifier)) {
				syntaxError("a struct name");
				return recover();
			}
			const auto& name = take();
			auto members = fields();
			if (not members.has_value()) {
				return recover();
			}
			accept(TokenKind::eSemicolon);

			if (findBuiltin(name.text) != nullptr || findStruct(name.text).has_value()) {
				return error(name, "Type '" + String(name.text) + "' is already declared.");
			}
			ir::TypeDesc desc {};
			desc.kind = ir::TypeKind::eStruct;
			desc.name = name.text;
			desc.fields = std::move(*members);
			_shared.module.types.push_back(std::move(desc));
		}

		void function(cref<Annotations> ann_) {
			take();
			if (not at(TokenKind::eIdentifier)) {
				syntaxError("a function name");
				return recover();
			}
			ir::FunctionDecl fn {};
			fn.name = take().text;
			fn.redefineAllowed = ann_.allow != nullptr;
			fn.when = ann_.binding.when;

			if (not expect(TokenKind::eLParen, "'('")) {
				return recover();
			}
			if (not accept(TokenKind::eRParen)) {
				do {
					if (not at(TokenKind::eIdentifier)) {
						syntaxError("a parameter name");
						return recover();
					}
					const auto& paramToken = take();
					String paramName { paramToken.text };
					if (std::ranges::find(fn.params, paramName, &ir::ParamDecl::name) != fn.params.end()) {
						error(paramToken, "Duplicate parameter '" + paramName + "'.");
					}
					if (not expect(TokenKind::eColon, "':' after the parameter name")) {
						return recover();
					}
					const auto paramType = type();
					if (not paramType.has_value()) {
						return recover();
					}
					fn.params.push_back(ir::ParamDecl { std::move(paramName), *paramType });
				} while (accept(TokenKind::eComma));
				if (not expect(TokenKind::eRParen, "')'")) {
					return recover();
				}
			}

			if (accept(TokenKind::eArrow)) {
				const auto ret = type();
				if (not ret.has_value()) {
					return recover();
				}
				fn.ret = *ret;
			}

			if (at(TokenKind::eLBrace)) {
				// parameters form the outermost scope; the body block opens its own
				_locals.clear();
				_scope = 0uz;
				for (const auto& param : fn.params) {
					_locals.emplace_back(param.name);
				}
				fn.body = ir::BodyRef { block() };
				_locals.clear();
			} else if (not expect(TokenKind::eSemicolon, "';' or a function body")) {
				return recover();
			}
			_shared.module.functions.push_back(std::move(fn));
		}

		/* `decl_` carries the annotations; type, localName and a defaulted id are filled here */
		void binding(ir::BindingDecl decl_) {
			auto& decl = decl_;
			if (accept(TokenKind::eKwBlock)) {
				if (not at(TokenKind::eIdentifier)) {
					syntaxError("a block name");
					return recover();
				}
				decl.localName = take().text;
				auto members = fields();
				if (not members.has_value()) {
					return recover();
				}
				accept(TokenKind::eSemicolon);

				// anonymous struct type: empty name, never interned or found by name
				ir::TypeDesc desc {};
				desc.kind = ir::TypeKind::eStruct;
				desc.fields = std::move(*members);
				_shared.module.types.push_back(std::move(desc));
				decl.type = ir::TypeRef { static_cast<u32>(_shared.module.types.size() - 1uz) };
			} else {
				const auto declType = type();
				if (not declType.has_value()) {
					return recover();
				}
				if (not at(TokenKind::eIdentifier)) {
					syntaxError("a binding name");
					return recover();
				}
				decl.type = *declType;
				decl.localName = take().text;
				if (not expect(TokenKind::eSemicolon, "';'")) {
					return recover();
				}
			}

			if (decl.id.empty()) {
				decl.id = decl.localName;
			}
			_shared.module.bindings.push_back(std::move(decl));
		}

		/*
		 * Bodies. Every statement recursion passes through statement(), every expression recursion through unary(),
		 * both under the shared Nest budget. Recursive frames stay small: node building and diagnostics live in
		 * non-recursive helpers. A parser recovers only from errors in its own tokens (fail()); a failed
		 * sub-statement has recovered already and is passed up as None.
		 */

		[[nodiscard]] ir::ExprRef expr(ir::Expr expr_) {
			auto& exprs = _shared.module.exprs;
			exprs.push_back(std::move(expr_));
			return ir::ExprRef { static_cast<u32>(exprs.size() - 1uz) };
		}

		[[nodiscard]] ir::ExprRef node(ir::ExprKind kind_, StringView name_, Vector<ir::ExprRef> operands_ = {}) {
			ir::Expr out {};
			out.kind = kind_;
			out.name = name_;
			out.operands = std::move(operands_);
			return expr(std::move(out));
		}

		[[nodiscard]] ir::StmtRef add(ir::Stmt stmt_) {
			auto& stmts = _shared.module.stmts;
			stmts.push_back(std::move(stmt_));
			return ir::StmtRef { static_cast<u32>(stmts.size() - 1uz) };
		}

		[[nodiscard]] ir::StmtRef stmt(ir::StmtKind kind_, ir::ExprRef lhs_ = {}, Vector<ir::StmtRef> children_ = {}) {
			ir::Stmt out {};
			out.kind = kind_;
			out.lhs = lhs_;
			out.children = std::move(children_);
			return add(std::move(out));
		}

		[[nodiscard]] Opt<ir::StmtRef> fail() {
			recover();
			return None;
		}

		void unterminated(cref<ir::Token> open_) {
			if (_eofErrors == 0uz && _shared.depthErrors == 0uL) {
				error(open_, "Unterminated block, expected '}'.");
			}
			++_eofErrors;
		}

		/* Statements */

		/* `{ statement* }`; the block is a scope */
		[[nodiscard]] ir::StmtRef block() {
			const auto& open = take();
			const Scope scope { _locals, _scope };
			Vector<ir::StmtRef> children {};
			while (not at(TokenKind::eRBrace) && not at(TokenKind::eEof)) {
				if (const auto child = statement(); child.has_value()) {
					children.push_back(*child);
				}
			}
			if (not accept(TokenKind::eRBrace)) {
				unterminated(open);
			}
			return stmt(ir::StmtKind::eBlock, {}, std::move(children));
		}

		/* consumes at least one token unless at `}` or the end of input */
		[[nodiscard]] Opt<ir::StmtRef> statement() {
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
				return letStatement();
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
		[[nodiscard]] Opt<ir::StmtRef> branch() {
			const Scope scope { _locals, _scope };
			return statement();
		}

		/* `@when (cond) statement` */
		[[nodiscard]] Opt<ir::StmtRef> whenStatement() {
			const auto head = whenHead();
			if (not head.has_value()) {
				return fail();
			}
			const auto child = branch();
			if (not child.has_value()) {
				return None;
			}
			_shared.module.stmts[head->index].children.push_back(*child);
			return *head;
		}

		/* keeps the condition out of the recursive frame */
		[[nodiscard]] Opt<ir::StmtRef> whenHead() {
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
		[[nodiscard]] Opt<ir::StmtRef> ifStatement() {
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

		[[nodiscard]] Opt<ir::StmtRef> returnStatement() {
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

		/* `let name [: Type] [= init];`, a type or an initializer required */
		[[nodiscard]] Opt<ir::StmtRef> letStatement() {
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
				error(name, "A 'let' declaration needs a type or an initializer.");
				return fail();
			}
			return local(name, declType.value_or(ir::TypeRef {}));
		}

		/* `Type name [= init];` */
		[[nodiscard]] Opt<ir::StmtRef> typedDeclaration() {
			const auto declType = type();
			if (not declType.has_value()) {
				return fail();
			}
			if (not at(TokenKind::eIdentifier)) {
				syntaxError("a local name");
				return fail();
			}
			return local(take(), *declType);
		}

		/* `[= init];` of both declaration forms (type unset = inferred later); the name is in scope after its
		 * initializer, even after an error */
		[[nodiscard]] Opt<ir::StmtRef> local(cref<ir::Token> name_, ir::TypeRef type_) {
			ir::Stmt decl {};
			decl.kind = ir::StmtKind::eDeclareLocal;
			decl.name = name_.text;
			decl.type = type_;
			bool valid = true;
			if (accept(TokenKind::eAssign)) {
				const auto init = expression();
				valid = init.has_value();
				decl.lhs = valid ? *init : decl.lhs;
			}
			valid = valid && expect(TokenKind::eSemicolon, "';'");

			if (std::find(_locals.begin() + static_cast<ptrdiff_t>(_scope), _locals.end(), name_.text) != _locals.end()) {
				error(name_, "Duplicate local '" + String(name_.text) + "'.");
			}
			_locals.push_back(name_.text);
			if (not valid) {
				return fail();
			}
			return add(std::move(decl));
		}

		/* `expr;` or `lvalue = expr;` */
		[[nodiscard]] Opt<ir::StmtRef> simpleStatement() {
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
		[[nodiscard]] Opt<ir::StmtRef> unsupported(cref<ir::Token> at_, StringView what_) {
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
		[[nodiscard]] bool compoundAhead() const {
			const auto kind = peek().kind;
			const auto ahead = next().kind;
			return precedenceOf(kind) >= additive && (ahead == TokenKind::eAssign || (kind == TokenKind::ePlus && ahead == kind));
		}

		/* a name, optionally behind field and index accesses */
		[[nodiscard]] bool assignable(ir::ExprRef ref_) const {
			const auto& exprs = _shared.module.exprs;
			const auto* e = &exprs[ref_.index];
			while ((e->kind == ir::ExprKind::eField || e->kind == ir::ExprKind::eIndex) && not e->operands.empty()) {
				e = &exprs[e->operands.front().index];
			}
			return e->kind == ir::ExprKind::eNameRef || (e->kind == ir::ExprKind::eBuiltin && contains(builtinVariables, e->name));
		}

		/* Expressions: comparisons < `+ -` < `* /` (precedence climbing) < unary `- !` < postfix `.name [i]` < primary */

		[[nodiscard]] Opt<ir::ExprRef> expression(u32 minPrecedence_ = 1uL) {
			auto lhs = unary();
			while (lhs.has_value() && precedenceOf(peek().kind) >= minPrecedence_ && not compoundAhead()) {
				const auto& op = take();
				const auto rhs = expression(precedenceOf(op.kind) + 1uL);
				lhs = rhs.has_value() ? Opt<ir::ExprRef> { node(ir::ExprKind::eBinary, op.text, { *lhs, *rhs }) } : None;
			}
			return lhs;
		}

		[[nodiscard]] Opt<ir::ExprRef> unary() {
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
				return node(ir::ExprKind::eUnary, op.text, { *operand });
			}
			return postfix(primary());
		}

		[[nodiscard]] Opt<ir::ExprRef> postfix(Opt<ir::ExprRef> base_) {
			while (base_.has_value()) {
				if (accept(TokenKind::eDot)) {
					if (not at(TokenKind::eIdentifier)) {
						syntaxError("a field name");
						return None;
					}
					// shortcut: no type inference, so any field name (struct member or swizzle) is accepted on any base;
					// upgrade when a typing pass exists (Task 8/9 need field types anyway)
					base_ = node(ir::ExprKind::eField, take().text, { *base_ });
				} else if (accept(TokenKind::eLBracket)) {
					const auto index = expression();
					if (not index.has_value() || not expect(TokenKind::eRBracket, "']'")) {
						return None;
					}
					base_ = node(ir::ExprKind::eIndex, {}, { *base_, *index });
				} else {
					break;
				}
			}
			return base_;
		}

		[[nodiscard]] Opt<ir::ExprRef> primary() {
			const auto& token = peek();
			if (token.kind == TokenKind::eInteger || token.kind == TokenKind::eFloat) {
				take();
				return node(ir::ExprKind::eLiteral, token.text);
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
		[[nodiscard]] Opt<ir::ExprRef> call(cref<ir::Token> name_) {
			take();
			Vector<ir::ExprRef> args {};
			if (not accept(TokenKind::eRParen)) {
				do {
					const auto arg = expression();
					if (not arg.has_value()) {
						return None;
					}
					args.push_back(*arg);
				} while (accept(TokenKind::eComma));
				if (not expect(TokenKind::eRParen, "',' or ')'")) {
					return None;
				}
			}
			return callee(name_, std::move(args));
		}

		/* type name -> construct, builtin function -> builtin, else a call resolved after the parse */
		[[nodiscard]] ir::ExprRef callee(cref<ir::Token> name_, Vector<ir::ExprRef> args_) {
			if (const auto type = namedType(name_.text); type.has_value()) {
				ir::Expr construct {};
				construct.kind = ir::ExprKind::eConstruct;
				construct.name = name_.text;
				construct.type = *type;
				construct.operands = std::move(args_);
				return expr(std::move(construct));
			}
			if (contains(builtinFunctions, name_.text)) {
				return node(ir::ExprKind::eBuiltin, name_.text, std::move(args_));
			}
			const auto pending = node(ir::ExprKind::eCall, name_.text, std::move(args_));
			_shared.calls.push_back(PendingCall { diagnostic(name_, "Unknown function '" + String(name_.text) + "'."), pending.index });
			return pending;
		}

		/* `true`/`false` -> literal; else local, then binding (by local name), then builtin variable */
		[[nodiscard]] ir::ExprRef name(cref<ir::Token> name_) {
			const auto text = name_.text;
			if (text == "true" || text == "false") {
				return node(ir::ExprKind::eLiteral, text);
			}

			ir::Expr named {};
			named.kind = ir::ExprKind::eNameRef;
			named.name = text;
			const auto& bindings = _shared.module.bindings;
			const auto binding = std::ranges::find(bindings, text, &ir::BindingDecl::localName);
			if (std::ranges::find(_locals, text) != _locals.end()) {
				named.local = true;
			} else if (binding != bindings.end()) {
				named.binding = ir::BindingRef { static_cast<u32>(binding - bindings.begin()) };
			} else if (contains(builtinVariables, text)) {
				named.kind = ir::ExprKind::eBuiltin;
			} else {
				error(name_, "Unknown name '" + String(text) + "'.");
			}
			// a local is never a binding use (usageOf relies on it)
			assert(not named.local || named.binding.index == ir::BindingRef {}.index);
			return expr(std::move(named));
		}
	};

	/* binds each call to a declaration of its name: the last one with a body (`@redefine`: the last wins), else the first
	 * shortcut: one declaration per name, so @when-variant bodies of a function all resolve to the last; upgrade when
	 * Task 6/7 merge and slice per variant. */
	void resolveCalls(ref<Shared> shared_) {
		auto& module = shared_.module;
		for (auto& pending : shared_.calls) {
			auto& call = module.exprs[pending.expr];
			Opt<u32> found = None;
			for (auto i = 0uz; i < module.functions.size(); ++i) {
				const auto& fn = module.functions[i];
				if (fn.name == call.name && (not found.has_value() || fn.body.has_value())) {
					found = static_cast<u32>(i);
				}
			}
			if (found.has_value()) {
				call.function = ir::FunctionRef { *found };
			} else {
				shared_.diags.emplace_back(std::move(pending.unknown));
			}
		}
	}

	void parseSource(StringView name_, StringView source_, ref<Shared> shared_) {
		auto tokens = ir::lex(source_);
		if (not tokens.has_value()) {
			for (auto& diag : tokens.error()) {
				diag.message = located(name_, not shared_.active.empty(), std::move(diag.message));
				shared_.diags.push_back(std::move(diag));
			}
			return;
		}

		shared_.active.emplace_back(name_);
		Parser { name_, *tokens, shared_ }.run();
		shared_.active.pop_back();
		shared_.done.emplace_back(name_);
	}
}

std::expected<ir::Module, ir::Diagnostics> ir::parseModule(
	StringView name_,
	StringView source_,
	EntryKind entry_,
	cref<ModuleResolver> includes_
) {
	Shared shared { includes_ };
	shared.module.name = name_;
	shared.module.entry = entry_;

	parseSource(name_, source_, shared);
	resolveCalls(shared);

	if (not shared.diags.empty()) {
		return std::unexpected(std::move(shared.diags));
	}
	return std::move(shared.module);
}
