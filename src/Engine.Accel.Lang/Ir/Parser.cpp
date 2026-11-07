#include "Parser.hpp"
#include "Typecheck.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <tuple>

#include "Builtin.hpp"
#include "Lexer.hpp"
#include "ParserImpl.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

namespace hg::engine::accel::lang::ir::detail {
	struct BuiltinType {
		StringView name;
		ir::TypeKind kind;
		ir::ScalarKind scalar;
		u16 bits;
		u16 n;
		u16 m;
		ir::TextureDim dim = ir::TextureDim::e2D;
	};

	constexpr std::array<BuiltinType, 7> fixedTypes {
		{
			{ "bool", ir::TypeKind::eScalar, ir::ScalarKind::eBool, 32u, 1u, 1u },
			{ "texture1d", ir::TypeKind::eTexture, ir::ScalarKind::eFloat, 32u, 1u, 1u, ir::TextureDim::e1D },
			{ "texture2d", ir::TypeKind::eTexture, ir::ScalarKind::eFloat, 32u, 1u, 1u },
			{ "texture2darray", ir::TypeKind::eTexture, ir::ScalarKind::eFloat, 32u, 1u, 1u, ir::TextureDim::e2DArray },
			{ "texture3d", ir::TypeKind::eTexture, ir::ScalarKind::eFloat, 32u, 1u, 1u, ir::TextureDim::e3D },
			{ "texturecube", ir::TypeKind::eTexture, ir::ScalarKind::eFloat, 32u, 1u, 1u, ir::TextureDim::eCube },
			{ "sampler", ir::TypeKind::eSampler, ir::ScalarKind::eFloat, 32u, 1u, 1u }
		}
	};

	/**
	 * `<k><bits>[vec<2-4> | mat<2-4>[x<2-4>]]` with k = f (16/32/64), s or u (8/16/32/64); a matrix is
	 * `matCxR` (C columns, R rows, `matC` is square). Matrices are float only: GLSL has no integer matrices.
	 * `intMatrix_` is set for the integer matrix spellings, which are recognized but not types.
	 */
	[[nodiscard]] static Opt<BuiltinType> findBuiltin(StringView name_, ptr<bool> intMatrix_ = nullptr) {
		if (const auto it = std::ranges::find(fixedTypes, name_, &BuiltinType::name); it != fixedTypes.end()) {
			return *it;
		}
		if (name_.size() < 2uz) {
			return None;
		}
		const auto scalar = name_[0] == 'f' ? ir::ScalarKind::eFloat : name_[0] == 's' ? ir::ScalarKind::eInt :
			name_[0] == 'u' ? ir::ScalarKind::eUInt : ir::ScalarKind::eBool;
		auto rest = name_.substr(1uz);
		u16 bits = 0u;
		while (not rest.empty() && rest.front() >= '0' && rest.front() <= '9' && bits < 100u) {
			bits = static_cast<u16>(bits * 10u + static_cast<u16>(rest.front() - '0'));
			rest.remove_prefix(1uz);
		}
		const auto knownBits = scalar == ir::ScalarKind::eFloat ? (bits == 16u || bits == 32u || bits == 64u) :
			(bits == 8u || bits == 16u || bits == 32u || bits == 64u);
		if (scalar == ir::ScalarKind::eBool || not knownBits) {
			return None;
		}
		const auto digit = [](char c_) { return c_ >= '2' && c_ <= '4' ? static_cast<u16>(c_ - '0') : u16 { 0u }; };
		if (rest.empty()) {
			return BuiltinType { name_, ir::TypeKind::eScalar, scalar, bits, 1u, 1u };
		}
		if (rest.starts_with("vec") && rest.size() == 4uz && digit(rest[3]) != 0u) {
			return BuiltinType { name_, ir::TypeKind::eVector, scalar, bits, digit(rest[3]), 1u };
		}
		const auto square = rest.size() == 4uz;
		const auto rect = rest.size() == 6uz && rest[4] == 'x' && digit(rest[5]) != 0u;
		if (rest.starts_with("mat") && (square || rect) && digit(rest[3]) != 0u) {
			if (scalar != ir::ScalarKind::eFloat) {
				if (intMatrix_ != nullptr) {
					*intMatrix_ = true;
				}
				return None;
			}
			return BuiltinType { name_, ir::TypeKind::eMatrix, scalar, bits, digit(rest[3]), square ? digit(rest[3]) : digit(rest[5]) };
		}
		return None;
	}

	[[nodiscard]] static bool isIntMatrixName(StringView name_) {
		auto intMatrix = false;
		std::ignore = findBuiltin(name_, &intMatrix);
		return intMatrix;
	}

	[[nodiscard]] static Opt<ir::BindMode> bindModeOf(StringView text_) {
		if (text_ == "forward") {
			return ir::BindMode::eForward;
		}
		if (text_ == "bind") {
			return ir::BindMode::eBind;
		}
		return None;
	}

	[[nodiscard]] static Opt<ir::BindClass> bindClassOf(StringView text_) {
		if (text_ == "uniform") {
			return ir::BindClass::eUniformBuffer;
		}
		if (text_ == "storage") {
			return ir::BindClass::eStorageBuffer;
		}
		if (text_ == "texture") {
			return ir::BindClass::eTexture;
		}
		if (text_ == "image") {
			return ir::BindClass::eStorageImage;
		}
		if (text_ == "sampler") {
			return ir::BindClass::eSampler;
		}
		if (text_ == "push") {
			return ir::BindClass::ePushConstant;
		}
		return None;
	}

	[[nodiscard]] static String describe(ref<const ir::Token> token_) {
		return token_.kind == TokenKind::eEof ? String("end of input") : "'" + String(token_.text) + "'";
	}

	[[nodiscard]] static ir::Condition conjoin(ir::Condition outer_, ir::Condition inner_) {
		if (outer_.kind == ir::ConditionKind::eTrue) {
			return inner_;
		}
		return ir::Condition::all({ std::move(outer_), std::move(inner_) });
	}

	/* One limit for every recursive production (conditions, @when blocks, includes, statements, expressions):
	 * bad input must not overflow the stack */
	constexpr u32 maxDepth = 256uL;

	static void parseSource(StringView name_, StringView source_, ref<Shared> shared_);

	[[nodiscard]] static String located(StringView module_, bool nested_, String message_) {
		return nested_ ? "in '" + String(module_) + "': " + message_ : message_;
	}

	void Parser::run() {
		while (not at(TokenKind::eEof)) {
			if (at(TokenKind::eRBrace)) {
				error(take(), "Unmatched '}'.");
				continue;
			}
			item(ir::Condition::always());
		}
	}

	bool Parser::tooDeep(ref<const ir::Token> at_) {
		if (_shared.depth <= maxDepth) {
			return false;
		}
		++_shared.depthErrors;
		error(at_, "Nesting too deep (limit " + std::to_string(maxDepth) + ").");
		return true;
	}

	ir::Diagnostic Parser::diagnostic(ref<const ir::Token> at_, String message_) const {
		return ir::Diagnostic {
			ir::Severity::eError, located(_name, nested(), std::move(message_)), at_.loc.line, at_.loc.column
		};
	}

	void Parser::error(ref<const ir::Token> at_, String message_) {
		if (at_.kind == TokenKind::eEof) {
			++_eofErrors;
		}
		_shared.diags.emplace_back(diagnostic(at_, std::move(message_)));
	}

	void Parser::syntaxError(StringView expected_) {
		error(peek(), "Expected " + String(expected_) + ", found " + describe(peek()) + ".");
	}

	bool Parser::expect(TokenKind kind_, StringView expected_) {
		if (accept(kind_)) {
			return true;
		}
		syntaxError(expected_);
		return false;
	}

	void Parser::recover() {
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

	void Parser::skipGroup(TokenKind open_, TokenKind close_) {
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

	void Parser::item(ir::Condition when_) {
		if (accept(TokenKind::eSemicolon)) {
			return;
		}
		if (at(TokenKind::eInclude)) {
			return include();
		}
		declaration(std::move(when_));
	}

	void Parser::include() {
		// allowed only while nothing but includes precede it (so also never inside a @when block)
		const bool allowed = std::all_of(
			_tokens.begin(),
			_tokens.begin() + static_cast<ptrdiff_t>(_pos),
			[](ref<const ir::Token> token_) {
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

	void Parser::declaration(ir::Condition when_) {
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
			} else if (name.text == "spec") {
				ann.spec = &name;
			} else if (name.text == "compute" || name.text == "fragment" || name.text == "geometry") {
				if (ann.entry != nullptr) {
					error(name, "Duplicate entry annotation.");
				}
				ann.entry = &name;
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
		if (head.kind == TokenKind::eKwConst) {
			if (ann.direction != nullptr || ann.redefine != nullptr || ann.binding.implicit) {
				error(head, "Only @spec and @when apply to a const declaration.");
			}
			return constantDecl(ann);
		}
		if (ann.spec != nullptr) {
			error(*ann.spec, "@spec applies to const declarations only.");
		}
		if (ann.entry != nullptr && head.kind != TokenKind::eKwFn) {
			error(*ann.entry, "@" + String(ann.entry->text) + " applies to fn main only.");
		}
		if (head.kind == TokenKind::eIdentifier && head.text == "workgroup" && next().kind == TokenKind::eIdentifier) {
			if (ann.direction != nullptr || ann.redefine != nullptr || ann.binding.implicit) {
				error(head, "Annotations do not apply to a workgroup declaration.");
			}
			return workgroupDecl(ann);
		}
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

	void Parser::whenBlock(ref<const ir::Condition> when_) {
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
	bool Parser::annotationArgs(ref<const ir::Token> annotation_, ref<Annotations> ann_) {
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
			if (annotation_.text == "compute" || annotation_.text == "fragment" || annotation_.text == "geometry") {
				if (not entryArg(annotation_, key, ann_)) {
					return false;
				}
				continue;
			}
			if (not at(TokenKind::eIdentifier) && not at(TokenKind::eString)) {
				syntaxError("an annotation value");
				return false;
			}
			annotationValue(annotation_, key, take(), ann_);
		} while (accept(TokenKind::eComma));
		return expect(TokenKind::eRParen, "')'");
	}

	/* value of `local_size = [x, y, z]` (@compute) or `early_fragment_tests = true|false` (@fragment); false after a syntax error */
	bool Parser::entryArg(ref<const ir::Token> annotation_, ref<const ir::Token> key_, ref<Annotations> ann_) {
		const bool compute = annotation_.text == "compute";
		if (key_.text == "local_size") {
			if (not expect(TokenKind::eLBracket, "'[' (local_size takes a list)")) {
				return false;
			}
			std::array<u32, 3> size { 1uL, 1uL, 1uL };
			size_t count = 0uz;
			do {
				if (not at(TokenKind::eInteger)) {
					syntaxError("a local_size component");
					return false;
				}
				const auto& component = take();
				u32 value = 0uL;
				for (const char digit : component.text) {
					value = value * 10uL + static_cast<u32>(digit - '0');
				}
				if (component.text.size() > 9uz || value == 0uL) {
					error(component, "Invalid local_size component '" + String(component.text) + "'.");
				}
				if (count < 3uz) {
					size[count] = value;
				} else if (count == 3uz) {
					error(component, "local_size takes at most three components.");
				}
				++count;
			} while (accept(TokenKind::eComma));
			if (not expect(TokenKind::eRBracket, "']'")) {
				return false;
			}
			if (not compute) {
				error(key_, "'local_size' applies to @compute only.");
			} else {
				ann_.attributes.localSize = size;
			}
			return true;
		}
		if (key_.text == "early_fragment_tests") {
			if (not at(TokenKind::eIdentifier)) {
				syntaxError("true or false");
				return false;
			}
			const auto& value = take();
			if (value.text != "true" && value.text != "false") {
				error(value, "Invalid value '" + String(value.text) + "' for annotation key 'early_fragment_tests'.");
			} else if (compute) {
				error(key_, "'early_fragment_tests' applies to @fragment only.");
			} else {
				ann_.attributes.earlyFragmentTests = value.text == "true";
			}
			return true;
		}
		if (annotation_.text == "geometry" &&
			(key_.text == "input" || key_.text == "output" || key_.text == "max_vertices" || key_.text == "invocations")) {
			if (key_.text == "max_vertices" || key_.text == "invocations") {
				if (not at(TokenKind::eInteger)) {
					syntaxError("an integer");
					return false;
				}
				const auto& number = take();
				u32 value = 0uL;
				for (const char digit : number.text) {
					value = value * 10uL + static_cast<u32>(digit - '0');
				}
				if (number.text.size() > 9uz || value == 0uL) {
					error(number, "Invalid " + String(key_.text) + " '" + String(number.text) + "'.");
				} else if (key_.text == "max_vertices") {
					ann_.attributes.maxVertices = value;
					ann_.geometryKeys |= 4u;
				} else {
					ann_.attributes.invocations = value;
				}
				return true;
			}
			if (not at(TokenKind::eIdentifier)) {
				syntaxError("a primitive name");
				return false;
			}
			const auto& value = take();
			using enum ir::Primitive;
			constexpr std::pair<StringView, ir::Primitive> names[] {
				{ "points", ePoints }, { "lines", eLines }, { "triangles", eTriangles }, { "lines_adjacency", eLinesAdjacency },
				{ "triangles_adjacency", eTrianglesAdjacency }, { "line_strip", eLineStrip }, { "triangle_strip", eTriangleStrip }
			};
			const auto it = std::ranges::find(names, value.text, &std::pair<StringView, ir::Primitive>::first);
			const bool isInput = key_.text == "input";
			const bool inputKind = it != std::end(names) && it->second <= eTrianglesAdjacency;
			const bool outputKind = it != std::end(names) && (it->second == ePoints || it->second >= eLineStrip);
			if (it == std::end(names) || (isInput ? not inputKind : not outputKind)) {
				error(value, "Invalid value '" + String(value.text) + "' for annotation key '" + String(key_.text) + "'.");
			} else if (isInput) {
				ann_.attributes.geometryInput = it->second;
				ann_.geometryKeys |= 1u;
			} else {
				ann_.attributes.geometryOutput = it->second;
				ann_.geometryKeys |= 2u;
			}
			return true;
		}
		error(key_, "Unknown annotation key '" + String(key_.text) + "' for @" + String(annotation_.text) + ".");
		// skip the value so one bad key does not cascade
		if (at(TokenKind::eLBracket)) {
			skipGroup(TokenKind::eLBracket, TokenKind::eRBracket);
		} else if (at(TokenKind::eIdentifier) || at(TokenKind::eInteger) || at(TokenKind::eString)) {
			take();
		}
		return true;
	}

	void Parser::annotationValue(
		ref<const ir::Token> annotation_,
		ref<const ir::Token> key_,
		ref<const ir::Token> value_,
		ref<Annotations> ann_
	) {
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
			if (valid && *cls == ir::BindClass::eStorageImage && _shared.module.entry != ir::EntryKind::eCompute) {
				error(value_, "storage images require a compute module.");
			}
		} else if (not redefine && key_.text == "access") {
			valid = word && (value_.text == "read" || value_.text == "write" || value_.text == "read_write");
			decl.access = valid ? (value_.text == "read" ? ir::Access::eRead : (value_.text == "write" ? ir::Access::eWrite : ir::Access::eReadWrite)) :
				decl.access;
		} else if (not redefine && key_.text == "packing") {
			valid = word && (value_.text == "std140" || value_.text == "std430");
			decl.packing = valid ? (value_.text == "std140" ? ir::Packing::eStd140 : ir::Packing::eStd430) : decl.packing;
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

	Opt<ir::Condition> Parser::parenCondition() {
		if (not expect(TokenKind::eLParen, "'(' after @when")) {
			return None;
		}
		auto cond = condition();
		if (not cond.has_value() || not expect(TokenKind::eRParen, "')'")) {
			return None;
		}
		return std::move(*cond);
	}

	Opt<ir::Condition> Parser::condition() {
		return chain(TokenKind::eOrOr, false);
	}

	/* `a op b op ...`: `||` over `&&` terms (conjunction_ = false), `&&` over unary terms (true) */
	Opt<ir::Condition> Parser::chain(TokenKind op_, bool conjunction_) {
		Vector<ir::Condition> terms {};
		do {
			auto term = conjunction_ ? unaryCondition() : chain(TokenKind::eAndAnd, true);
			if (not term.has_value()) {
				return None;
			}
			terms.emplace_back(std::move(*term));
		} while (accept(op_));

		if (terms.size() == 1uz) {
			return std::move(terms.front());
		}
		return conjunction_ ? ir::Condition::all(std::move(terms)) : ir::Condition::any(std::move(terms));
	}

	/* every condition recursion (`!`, `( )`, any/all/none arguments) passes through here */
	Opt<ir::Condition> Parser::unaryCondition() {
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
	Opt<ir::Condition> Parser::combinator(ref<const ir::Token> name_) {
		take();
		Vector<ir::Condition> args {};
		do {
			auto arg = condition();
			if (not arg.has_value()) {
				return None;
			}
			args.emplace_back(std::move(*arg));
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

	void Parser::unknownCombinator(ref<const ir::Token> name_) {
		error(name_, "Unknown condition '" + String(name_.text) + "', expected any, all or none.");
	}

	/* leaf `Toggle == Value` / `Toggle != Value`; never recurses */
	Opt<ir::Condition> Parser::comparison(ref<const ir::Token> name) {
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

	ir::TypeRef Parser::intern(ir::TypeDesc desc_) {
		auto& types = _shared.module.types;
		const auto it = std::ranges::find_if(
			types,
			[&](ref<const ir::TypeDesc> type_) {
				return type_.kind != ir::TypeKind::eStruct && type_.kind == desc_.kind && type_.scalar == desc_.scalar &&
					type_.bits == desc_.bits && type_.n == desc_.n && type_.m == desc_.m &&
					type_.element == desc_.element && type_.count == desc_.count && type_.dim == desc_.dim &&
					type_.name == desc_.name;
			}
		);
		if (it != types.end()) {
			return ir::TypeRef { static_cast<u32>(it - types.begin()) };
		}
		types.emplace_back(std::move(desc_));
		return ir::TypeRef { static_cast<u32>(types.size() - 1uz) };
	}

	Opt<ir::TypeRef> Parser::findStruct(StringView name_) const {
		const auto& types = _shared.module.types;
		const auto it = std::ranges::find_if(
			types,
			[&](ref<const ir::TypeDesc> type_) {
				return type_.kind == ir::TypeKind::eStruct && not type_.name.empty() && type_.name == name_;
			}
		);
		if (it == types.end()) {
			return None;
		}
		return ir::TypeRef { static_cast<u32>(it - types.begin()) };
	}

	bool Parser::isTypeName(StringView name_) const {
		return findBuiltin(name_).has_value() || isIntMatrixName(name_) || findStruct(name_).has_value();
	}

	/* a struct or builtin type by name, interned */
	Opt<ir::TypeRef> Parser::namedType(StringView name_) {
		if (const auto builtin = findBuiltin(name_); builtin.has_value()) {
			ir::TypeDesc desc {};
			desc.kind = builtin->kind;
			desc.scalar = builtin->scalar;
			desc.bits = builtin->bits;
			desc.n = builtin->n;
			desc.m = builtin->m;
			desc.dim = builtin->dim;
			return intern(std::move(desc));
		}
		return findStruct(name_);
	}

	/* `Name` or `Name[N]` / `Name[]` (repeatable); reports and returns None on failure */
	Opt<ir::TypeRef> Parser::type() {
		if (not at(TokenKind::eIdentifier)) {
			syntaxError("a type");
			return None;
		}
		const auto& name = take();

		Opt<ir::TypeRef> result = None;
		if (name.text == "image2d") {
			// `image2d<format>`
			if (not expect(TokenKind::eLess, "'<' after image2d")) {
				return None;
			}
			if (not at(TokenKind::eIdentifier)) {
				syntaxError("an image format");
				return None;
			}
			const auto& format = take();
			if (format.text != "rgba8" && format.text != "rgba16f" && format.text != "r32f" && format.text != "r32ui") {
				error(format, "Unknown image format '" + String(format.text) + "', expected rgba8, rgba16f, r32f or r32ui.");
				return None;
			}
			if (not expect(TokenKind::eGreater, "'>'")) {
				return None;
			}
			ir::TypeDesc desc {};
			desc.kind = ir::TypeKind::eImage;
			desc.dim = ir::TextureDim::e2D;
			desc.name = format.text;
			result = intern(std::move(desc));
		} else {
			result = namedType(name.text);
		}
		if (not result.has_value()) {
			error(
				name,
				isIntMatrixName(name.text) ?
				"GLSL has no integer matrix type '" + String(name.text) + "'; matrices are float only." :
				"Unknown type '" + String(name.text) + "'."
			);
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
	Opt<Vector<ir::FieldDesc>> Parser::fields() {
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
					out.emplace_back(ir::FieldDesc { std::move(name), *fieldType });
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

	void Parser::toggleDecl() {
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
				values.emplace_back(&take());
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
		toggles.emplace_back(std::move(toggle));
	}

	void Parser::structDecl() {
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

		if (findBuiltin(name.text).has_value() || isIntMatrixName(name.text) || findStruct(name.text).has_value()) {
			return error(name, "Type '" + String(name.text) + "' is already declared.");
		}
		ir::TypeDesc desc {};
		desc.kind = ir::TypeKind::eStruct;
		desc.name = name.text;
		desc.fields = std::move(*members);
		_shared.module.types.emplace_back(std::move(desc));
	}

	void Parser::function(ref<const Annotations> ann_) {
		take();
		if (not at(TokenKind::eIdentifier)) {
			syntaxError("a function name");
			return recover();
		}
		ir::FunctionDecl fn {};
		fn.loc = peek().loc;
		fn.name = take().text;
		fn.redefineAllowed = ann_.allow != nullptr;
		fn.when = ann_.binding.when;

		if (ann_.entry != nullptr) {
			const bool compute = ann_.entry->text == "compute";
			const bool geometry = ann_.entry->text == "geometry";
			const auto expected = compute ? ir::EntryKind::eCompute : geometry ? ir::EntryKind::eGeometry : ir::EntryKind::eFragment;
			if (_shared.module.entry != expected) {
				error(*ann_.entry, "@" + String(ann_.entry->text) + " contradicts the module's stage.");
			} else if (fn.name != "main") {
				error(*ann_.entry, "@" + String(ann_.entry->text) + " applies to fn main only.");
			} else {
				if (geometry && ann_.geometryKeys != 7u) {
					error(*ann_.entry, "@geometry requires input, output and max_vertices.");
				}
				_shared.module.attributes = ann_.attributes;
			}
		}

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
				if (paramName.starts_with("_hg_")) {
					error(paramToken, "Identifiers starting with '_hg_' are reserved.");
				}
				auto kind = ir::ParamKind::eValue;
				if (accept(TokenKind::eAmp)) {
					kind = accept(TokenKind::eKwMut) ? ir::ParamKind::eRefMut : ir::ParamKind::eRef;
				}
				const auto paramType = type();
				if (not paramType.has_value()) {
					return recover();
				}
				fn.params.emplace_back(ir::ParamDecl { std::move(paramName), *paramType, kind });
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
		_shared.module.functions.emplace_back(std::move(fn));
	}

	/* `const NAME: T = expr;` with an optional `@spec (id = "...")`; the initializer is checked by typecheck */
	void Parser::constantDecl(ref<const Annotations> ann_) {
		take();
		if (not at(TokenKind::eIdentifier)) {
			syntaxError("a constant name");
			return recover();
		}
		ir::ConstantDecl decl {};
		decl.loc = peek().loc;
		const auto& nameToken = take();
		decl.name = nameToken.text;
		decl.when = ann_.binding.when;
		if (ann_.spec != nullptr) {
			if (ann_.binding.id.empty()) {
				error(*ann_.spec, "@spec requires (id = \"...\").");
			} else {
				decl.specId = ann_.binding.id;
			}
		}
		if (not expect(TokenKind::eColon, "':'")) {
			return recover();
		}
		const auto declType = type();
		if (not declType.has_value() || not expect(TokenKind::eAssign, "'='")) {
			return recover();
		}
		decl.type = *declType;
		const auto init = expression();
		if (not init.has_value()) {
			return recover();
		}
		decl.value = *init;
		if (not expect(TokenKind::eSemicolon, "';'")) {
			return recover();
		}
		auto& constants = _shared.module.constants;
		if (std::ranges::find(constants, decl.name, &ir::ConstantDecl::name) != constants.end()) {
			return error(nameToken, "Duplicate constant '" + decl.name + "'.");
		}
		constants.emplace_back(std::move(decl));
	}

	/* `workgroup name: T;` */
	void Parser::workgroupDecl(ref<const Annotations> ann_) {
		const auto& head = take();
		if (_shared.module.entry != ir::EntryKind::eCompute) {
			error(head, "workgroup variables require a compute module.");
		}
		const auto& nameToken = take();
		if (not expect(TokenKind::eColon, "':'")) {
			return recover();
		}
		const auto declType = type();
		if (not declType.has_value() || not expect(TokenKind::eSemicolon, "';'")) {
			return recover();
		}
		ir::BindingDecl decl {};
		decl.localName = nameToken.text;
		decl.id = decl.localName;
		decl.type = *declType;
		decl.bindClass = ir::BindClass::eWorkgroup;
		decl.input = false;
		decl.when = ann_.binding.when;
		decl.access = ir::Access::eReadWrite;
		_shared.module.bindings.emplace_back(std::move(decl));
	}

	/* `decl_` carries the annotations; type, localName and a defaulted id are filled here */
	void Parser::binding(ir::BindingDecl decl_) {
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
			_shared.module.types.emplace_back(std::move(desc));
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
			{
				const auto& declared = _shared.module.types[declType->index];
				const auto entry = _shared.module.entry;
				if (declared.kind == ir::TypeKind::eArray && declared.count == 0uL && decl.input && decl.mode == ir::BindMode::eForward &&
					(entry == ir::EntryKind::eGeometry || entry == ir::EntryKind::eTessCtrl || entry == ir::EntryKind::eTessEval)) {
					decl.arrayed = true;
					decl.type = declared.element;
				}
			}
			const auto nameToken = take();
			decl.localName = nameToken.text;
			const auto kind = _shared.module.types[declType->index].kind;
			if (decl.packing != ir::Packing::eDefault && (kind == ir::TypeKind::eTexture || kind == ir::TypeKind::eSampler || kind == ir::TypeKind::eImage)) {
				error(nameToken, "packing is not allowed on opaque binding '" + decl.localName + "'.");
			}
			if (not expect(TokenKind::eSemicolon, "';'")) {
				return recover();
			}
		}

		if (decl.id.empty()) {
			decl.id = decl.localName;
		}
		_shared.module.bindings.emplace_back(std::move(decl));
	}

	/* binds each call to a declaration of its name (typecheck then picks the overload by argument types)
	 */
	static void resolveCalls(ref<Shared> shared_) {
		auto& module = shared_.module;
		/* a user function shadows a builtin function of the same name */
		for (auto& e : module.exprs) {
			if (e.kind != ir::ExprKind::eBuiltin || ir::findBuiltinFunction(e.name) == nullptr) {
				continue;
			}
			for (auto i = 0uz; i < module.functions.size(); ++i) {
				if (module.functions[i].name == e.name) {
					e.kind = ir::ExprKind::eCall;
					e.function = ir::FunctionRef { static_cast<u32>(i) };
				}
			}
		}
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

	static void parseSource(StringView name_, StringView source_, ref<Shared> shared_) {
		auto tokens = ir::lex(source_);
		if (not tokens.has_value()) {
			for (auto& diag : tokens.error()) {
				diag.message = located(name_, not shared_.active.empty(), std::move(diag.message));
				shared_.diags.emplace_back(std::move(diag));
			}
			return;
		}

		shared_.active.emplace_back(name_);
		Parser { name_, *tokens, shared_ }.run();
		shared_.active.pop_back();
		shared_.done.emplace_back(name_);
	}

	[[nodiscard]] static std::expected<ir::Module, ir::Diagnostics> parseWhole(
		StringView name_,
		StringView source_,
		ir::EntryKind entry_,
		ref<const ir::ModuleResolver> includes_
	) {
		Shared shared { includes_ };
		shared.module.name = name_;
		shared.module.entry = entry_;

		parseSource(name_, source_, shared);
		resolveCalls(shared);
		if (shared.diags.empty()) {
			shared.diags = ir::typecheck(shared.module);
		}

		const auto failed = std::ranges::any_of(shared.diags, [](const auto& d_) { return d_.severity == ir::Severity::eError; });
		if (failed) {
			return std::unexpected(std::move(shared.diags));
		}
		shared.module.warnings = std::move(shared.diags);
		return std::move(shared.module);
	}
}

std::expected<ir::Module, ir::Diagnostics> ir::parseModule(
	StringView name_,
	StringView source_,
	EntryKind entry_,
	ref<const ModuleResolver> includes_
) {
	return detail::parseWhole(name_, source_, entry_, includes_);
}
