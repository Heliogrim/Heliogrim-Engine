#include "Typecheck.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <limits>
#include <string>
#include <utility>

#include "Builtin.hpp"
#include "Place.hpp"
#include "Uniformity.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	constexpr u32 maxDepth = 256uL;
	constexpr u32 describeDepthLimit = 8uL;

	using ir::ExprKind;
	using ir::ExprRef;
	using ir::ScalarKind;
	using ir::StmtKind;
	using ir::TypeDesc;
	using ir::TypeKind;
	using ir::TypeRef;

	[[nodiscard]] bool isComparison(ir::Op op_) {
		return op_ == ir::Op::eLt || op_ == ir::Op::eLe || op_ == ir::Op::eGt || op_ == ir::Op::eGe;
	}

	class Checker {
	public:
		explicit Checker(ref<ir::Module> mod_) :
			_mod(mod_),
			_hasMain(
				std::ranges::any_of(
					mod_.functions,
					[](ref<const ir::FunctionDecl> fn_) { return fn_.name == "main" && fn_.body.has_value(); }
				)
			) {}

	public:
		[[nodiscard]] ir::Diagnostics run() {
			for (auto i = 0uz; i < _mod.functions.size(); ++i) {
				for (auto j = i + 1uz; j < _mod.functions.size(); ++j) {
					const auto& a = _mod.functions[i];
					const auto& b = _mod.functions[j];
					if (a.name == b.name && a.params.size() == b.params.size() && ir::signatureOf(_mod, a) == ir::signatureOf(_mod, b) &&
						not std::ranges::equal(a.params, b.params, {}, &ir::ParamDecl::kind, &ir::ParamDecl::kind)) {
						const Here here { _loc, b.loc };
						error(codes::asl("duplicate-declaration"), "overloads of '" + a.name + "' differ only in reference kind");
					}
				}
			}

			for (auto i = 0uz; i < _mod.constants.size(); ++i) {
				constant(i);
			}

			for (auto i = 0uz; i < _mod.functions.size(); ++i) {
				if (not _mod.functions[i].body.has_value()) {
					continue;
				}

				const auto root = _mod.functions[i].body->root;
				_ret = _mod.functions[i].ret;
				_scope.clear();
				_loops = 0uL;
				for (const auto& param : _mod.functions[i].params) {
					const auto mode = param.kind == ir::ParamKind::eRefMut ?
						Mode::eRefWrite :
						(param.kind == ir::ParamKind::eRef ? Mode::eRefRead : Mode::eConstant);
					_scope.emplace_back(param.name, param.type, mode);
				}

				stmt(root);
			}

			literalRanges();
			return std::move(_diags);
		}

	private:
		struct Nest {
			explicit Nest(ref<u32> depth_) :
				_depth(depth_) {
				++_depth;
			}

			~Nest() {
				--_depth;
			}

			ref<u32> _depth;
		};

		enum class Literal : u8 {
			eNone,
			eInt,
			eFloat
		};

		/* what a name in scope allows: assignment (mutable local, `&mut`) or only reads */
		enum class Mode : u8 {
			eMutable,
			eConstant,
			eRefRead,
			eRefWrite
		};

		struct Entry {
			String name;
			TypeRef type;
			Mode mode;
			/* for an alias: the root name it borrows */
			String target {};
			/* for an alias of a local: scope index of that local (stable, aliases always follow their target); ~0 for bindings */
			size_t targetIndex = ~0uz;
			/* for an alias: the borrowed place, aliases expanded */
			Opt<ir::PlacePath> place {};
			/* for an alias rooted in another alias (re-borrow): scope index of that alias */
			size_t parentAlias = ~0uz;
			/* for a local: its `let` / `const` keyword, empty when it has none */
			ir::SourceLoc keyword {};
		};

		ref<ir::Module> _mod;
		/* library modules (no `fn main`) are stage-checked after merge, see checkStageValidity */
		bool _hasMain;
		ir::Diagnostics _diags;
		Vector<Entry> _scope;
		TypeRef _ret;
		u32 _depth = 0uL;
		u32 _loops = 0uL;
		/* range expressions whose context is a borrow (alias initializer or call argument) */
		Vector<u32> _borrowed;
		/* lhs of the compound assignment being checked: its conversion help goes on the rhs, the target cannot change */
		size_t _compound = ~0uz;

		/* head token of the node being checked; restored on scope exit so errors after operand recursion stay on the parent */
		ir::SourceLoc _loc {};

		struct Here {
			explicit Here(ref<ir::SourceLoc> slot_, ir::SourceLoc next_) :
				_slot(slot_),
				_saved(slot_) { _slot = next_; }

			~Here() { _slot = _saved; }

			ref<ir::SourceLoc> _slot;
			ir::SourceLoc _saved;
		};

		void error(ref<const ir::DiagnosticCodeInfo> code_, String message_) {
			_diags.emplace_back(ir::Diagnostic::error(code_, std::move(message_)).at(spanIn(_mod, _loc)));
		}
		void error(ref<const ir::DiagnosticCodeInfo> code_, String message_, String note_) {
			_diags.emplace_back(ir::Diagnostic::error(code_, std::move(message_)).at(spanIn(_mod, _loc)).note(std::move(note_)));
		}

		[[nodiscard]] String name(TypeRef type_) const {
			return ir::describeType(_mod.types, type_);
		}

		[[nodiscard]] bool valid(TypeRef type_) const {
			return type_.index < _mod.types.size();
		}

		/* Type table */

		[[nodiscard]] static bool sameDesc(ref<const TypeDesc> a_, ref<const TypeDesc> b_) {
			if (a_.kind != b_.kind) {
				return false;
			}

			switch (a_.kind) {
				case TypeKind::eScalar: return a_.scalar == b_.scalar && a_.bits == b_.bits;
				case TypeKind::eVector: return a_.scalar == b_.scalar && a_.bits == b_.bits && a_.n == b_.n;
				case TypeKind::eMatrix: return a_.scalar == b_.scalar && a_.bits == b_.bits && a_.n == b_.n && a_.m == b_.m;
				case TypeKind::eStruct: return a_.name == b_.name && std::ranges::equal(
						a_.fields,
						b_.fields,
						[](ref<const ir::FieldDesc> l_, ref<const ir::FieldDesc> r_) { return l_.name == r_.name && l_.type == r_.type; }
					);
				case TypeKind::eTexture: return a_.dim == b_.dim && a_.scalar == b_.scalar;
				case TypeKind::eImage: return a_.dim == b_.dim && a_.name == b_.name;
				case TypeKind::eArray: return a_.count == b_.count && a_.element == b_.element;
				case TypeKind::eRegion: return a_.element == b_.element;
				default: return true;
			}
		}

		[[nodiscard]] bool same(TypeRef a_, TypeRef b_, u32 depth_ = 0uL) const {
			if (a_ == b_) {
				return valid(a_);
			}

			if (not valid(a_) || not valid(b_) || depth_ > describeDepthLimit) {
				return false;
			}

			const auto& a = _mod.types[a_.index];
			const auto& b = _mod.types[b_.index];
			if (a.kind == TypeKind::eArray && b.kind == TypeKind::eArray) {
				return a.count == b.count && same(a.element, b.element, depth_ + 1uL);
			}
			return sameDesc(a, b);
		}

		[[nodiscard]] TypeRef intern(TypeDesc desc_) {
			for (auto i = 0uz; i < _mod.types.size(); ++i) {
				if (sameDesc(_mod.types[i], desc_)) {
					return TypeRef { static_cast<u32>(i) };
				}
			}

			_mod.types.emplace_back(std::move(desc_));
			return TypeRef { static_cast<u32>(_mod.types.size() - 1uz) };
		}

		[[nodiscard]] TypeRef scalarOf(ScalarKind kind_, u32 bits_) {
			TypeDesc desc {};
			desc.kind = TypeKind::eScalar;
			desc.scalar = kind_;
			desc.bits = bits_;
			return intern(std::move(desc));
		}

		[[nodiscard]] TypeRef vectorOf(ScalarKind kind_, u32 bits_, u32 n_) {
			if (n_ == 1uL) {
				return scalarOf(kind_, bits_);
			}

			TypeDesc desc {};
			desc.kind = TypeKind::eVector;
			desc.scalar = kind_;
			desc.bits = bits_;
			desc.n = n_;
			return intern(std::move(desc));
		}

		[[nodiscard]] TypeRef boolType() {
			return scalarOf(ScalarKind::eBool, 32uL);
		}

		/* Scalar component type of a scalar, vector or matrix */
		[[nodiscard]] TypeRef componentOf(TypeRef type_) {
			if (not valid(type_)) {
				return {};
			}
			const auto desc = _mod.types[type_.index];
			return desc.kind == TypeKind::eScalar || desc.kind == TypeKind::eVector || desc.kind == TypeKind::eMatrix ?
				scalarOf(desc.scalar, desc.bits) :
				TypeRef {};
		}

		[[nodiscard]] bool numeric(TypeRef type_) const {
			if (not valid(type_)) {
				return false;
			}

			const auto& desc = _mod.types[type_.index];
			return (desc.kind == TypeKind::eScalar || desc.kind == TypeKind::eVector || desc.kind == TypeKind::eMatrix) &&
				desc.scalar != ScalarKind::eBool && (desc.kind != TypeKind::eMatrix || desc.scalar == ScalarKind::eFloat);
		}

		[[nodiscard]] bool isKind(TypeRef type_, TypeKind kind_) const {
			return valid(type_) && _mod.types[type_.index].kind == kind_;
		}

		[[nodiscard]] bool isBool(TypeRef type_) const {
			return isKind(type_, TypeKind::eScalar) && _mod.types[type_.index].scalar == ScalarKind::eBool;
		}

		[[nodiscard]] bool isInteger(TypeRef type_) const {
			if (not isKind(type_, TypeKind::eScalar)) {
				return false;
			}

			const auto scalar = _mod.types[type_.index].scalar;
			return scalar == ScalarKind::eInt || scalar == ScalarKind::eUInt;
		}

		/* Literal adaptation (D4): a plain integer literal takes any numeric scalar type, a float literal only float types */

		[[nodiscard]] Literal literalOf(ExprRef ref_) const {
			if (ref_.index >= _mod.exprs.size()) {
				return Literal::eNone;
			}

			const auto& e = _mod.exprs[ref_.index];
			if (e.kind == ExprKind::eUnary && e.op == ir::Op::eNeg && e.operands.size() == 1uz) {
				return literalOf(e.operands.front());
			}

			if (e.kind != ExprKind::eLiteral) {
				return Literal::eNone;
			}

			// a plain integer (s64) adapts; unsigned (u64) and bool literals keep their type
			if (std::holds_alternative<f64>(e.literal.value)) {
				return Literal::eFloat;
			}

			return std::holds_alternative<s64>(e.literal.value) ? Literal::eInt : Literal::eNone;
		}

		[[nodiscard]] bool adaptable(ExprRef ref_, TypeRef target_) const {
			if (not isKind(target_, TypeKind::eScalar)) {
				return false;
			}

			const auto scalar = _mod.types[target_.index].scalar;
			switch (literalOf(ref_)) {
				case Literal::eInt: return scalar != ScalarKind::eBool;
				case Literal::eFloat: return scalar == ScalarKind::eFloat;
				default: return false;
			}
		}

		/* an integer literal fits when its value is exact in the target type */
		[[nodiscard]] static bool fits(const u64 value_, ref<const TypeDesc> to_, const bool negated_) {
			switch (to_.scalar) {
				case ScalarKind::eInt: return value_ <= (1uLL << (to_.bits - 1uL)) - (negated_ ? 0uLL : 1uLL);
				case ScalarKind::eUInt: return negated_ ? value_ == 0uLL : to_.bits >= 64uL || value_ < (1uLL << to_.bits);
				case ScalarKind::eFloat: {
					const auto mantissa = to_.bits == 16uL ? 11 : to_.bits == 32uL ? 24 : 53;
					return value_ == 0uLL || (
						std::bit_width(value_ >> std::countr_zero(value_)) <= mantissa && (to_.bits != 16uL || value_ <= 65504uLL)
					);
				}
				default: return true;
			}
		}

		void retype(ExprRef ref_, TypeRef type_) {
			auto& e = _mod.exprs[ref_.index];
			e.type = type_;
			if (e.kind == ExprKind::eUnary && e.operands.size() == 1uz) {
				retype(e.operands.front(), type_);
			}
		}

		/* every typed literal against its final type, after context has settled it (default, coerced, or `u`-suffixed) */
		void literalRanges() {
			Vector<bool> negated(_mod.exprs.size(), false);
			for (const auto& e : _mod.exprs) {
				if (e.kind == ExprKind::eUnary && e.op == ir::Op::eNeg && e.operands.size() == 1uz && e.operands.front().index < negated.size()) {
					negated[e.operands.front().index] = true;
				}
			}

			for (auto i = 0uz; i < _mod.exprs.size(); ++i) {
				const auto& e = _mod.exprs[i];
				if (e.kind != ExprKind::eLiteral || not valid(e.type) || not isKind(e.type, TypeKind::eScalar)) {
					continue;
				}

				const auto& to = _mod.types[e.type.index];
				const auto& value = e.literal.value;
				bool ok = true;
				String spelled;
				if (std::holds_alternative<s64>(value) || std::holds_alternative<u64>(value)) {
					const auto v = std::holds_alternative<s64>(value) ? static_cast<u64>(std::get<s64>(value)) : std::get<u64>(value);
					ok = fits(v, to, negated[i]);
					if (spelled.empty()) { spelled = std::to_string(v); }
				} else if (std::holds_alternative<f64>(value) && to.scalar == ScalarKind::eFloat) {
					const auto v = std::abs(std::get<f64>(value));
					ok = to.bits == 16uL ? v <= 65504.0 : to.bits == 32uL ? v <= static_cast<f64>(std::numeric_limits<f32>::max()) : true;
					if (spelled.empty()) { spelled = std::to_string(std::get<f64>(value)); }
				}

				if (not ok) {
					_diags.emplace_back(
						ir::Diagnostic::error(
							codes::asl("literal-out-of-range"),
							"literal '" + String { negated[i] ? "-" : "" } + spelled + "' does not fit " + ir::quotedType(_mod.types, e.type)
						).at(spanIn(_mod, e.loc))
					);
				}
			}
		}

		[[nodiscard]] bool coerce(ExprRef ref_, TypeRef target_) {
			if (same(_mod.exprs[ref_.index].type, target_)) {
				return true;
			}

			if (adaptable(ref_, target_)) {
				retype(ref_, target_);
				return true;
			}
			return false;
		}

		/* numeric scalars, or vectors of one width, that differ only in their component: ASL never converts these implicitly */
		[[nodiscard]] bool conversion(TypeRef from_, TypeRef to_) const {
			if (not numeric(from_) || not numeric(to_)) {
				return false;
			}

			const auto& f = _mod.types[from_.index];
			const auto& t = _mod.types[to_.index];
			return f.kind == t.kind && f.kind != TypeKind::eMatrix && (f.kind == TypeKind::eScalar || f.n == t.n) && not sameDesc(f, t);
		}

		/* `from_` cannot be used as `to_`; a numeric conversion gets the help that writes it out */
		void incompatible(String message_, ExprRef from_, TypeRef fromType_, TypeRef toType_) {
			if (not conversion(fromType_, toType_)) {
				error(codes::asl("type-mismatch"), std::move(message_));
				return;
			}

			const auto* e = from_.index < _mod.exprs.size() ? &_mod.exprs[from_.index] : nullptr;
			const auto* lit = e != nullptr && e->kind == ExprKind::eUnary && e->op == ir::Op::eNeg && e->operands.size() == 1uz &&
				e->operands.front().index < _mod.exprs.size() ? &_mod.exprs[e->operands.front().index] : e;
			if (lit != nullptr && lit->kind == ExprKind::eLiteral && std::holds_alternative<f64>(lit->literal.value) &&
				_mod.types[toType_.index].scalar != ScalarKind::eFloat) {
				error(
					codes::asl("literal-out-of-range"),
					"literal '" + String { lit != e ? "-" : "" } + ir::literalText(lit->literal) + "' does not fit " + ir::quotedType(_mod.types, toType_),
					"write an integer literal"
				);
				return;
			}

			const auto named = e != nullptr && e->kind == ExprKind::eNameRef && e->loc.length != 0uL;
			const auto spelled = named ? e->name :
				e != nullptr && e->kind == ExprKind::eLiteral ? ir::literalText(e->literal) :
				String { "..." };
			const auto written = name(toType_) + "(" + spelled + ")";

			auto diagnostic = ir::Diagnostic::error(codes::asl("implicit-conversion"), std::move(message_)).at(spanIn(_mod, _loc));
			std::move(diagnostic).fix(
				engine::diag::Fix {
					"write `" + written + "`",
					named ? Vector<engine::diag::TextEdit> { engine::diag::TextEdit { spanIn(_mod, e->loc), written } } : Vector<engine::diag::TextEdit> {},
					named ? engine::diag::Applicability::eMachineApplicable : engine::diag::Applicability::eHasPlaceholders
				}
			);
			_diags.emplace_back(std::move(diagnostic));
		}

		/* Statements */

		void stmt(ir::StmtRef ref_) {
			if (ref_.index >= _mod.stmts.size() || _depth > maxDepth) {
				return;
			}

			const Nest nest { _depth };
			const Here here { _loc, _mod.stmts[ref_.index].loc };
			const auto kind = _mod.stmts[ref_.index].kind;
			const auto lhs = _mod.stmts[ref_.index].lhs;
			const auto rhs = _mod.stmts[ref_.index].rhs;
			const auto children = _mod.stmts[ref_.index].children;

			switch (kind) {
				case StmtKind::eDeclareLocal: {
					const auto mutableLocal = _mod.stmts[ref_.index].mutable_;
					if (not mutableLocal && lhs.index >= _mod.exprs.size()) {
						error(codes::asl("incomplete-local-declaration"), "a 'const' local needs an initializer");
					}

					const auto declared = _mod.stmts[ref_.index].type;
					auto type = declared;
					if (lhs.index < _mod.exprs.size()) {
						const auto init = expr(lhs);
						if (valid(declared)) {
							if (valid(init) && not coerce(lhs, declared)) {
								incompatible(
									"cannot initialize '" + _mod.stmts[ref_.index].name + "' of type " + ir::quotedType(
										_mod.types,
										declared
									) + " with " + ir::quotedType(_mod.types, init),
									lhs,
									init,
									declared
								);
							}

						} else {
							type = init;
						}
					}

					_mod.stmts[ref_.index].type = type;
					_scope.emplace_back(_mod.stmts[ref_.index].name, type, mutableLocal ? Mode::eMutable : Mode::eConstant);
					_scope.back().keyword = _mod.stmts[ref_.index].keyword;
					break;
				}
				case StmtKind::eDeclareRef: {
					if (lhs.index < _mod.exprs.size() && _mod.exprs[lhs.index].kind == ExprKind::eRange) {
						_borrowed.emplace_back(lhs.index);
					}

					const auto type = expr(lhs);
					const auto writes = _mod.stmts[ref_.index].mutable_;
					if (lhs.index >= _mod.exprs.size() || not ir::isPlace(_mod, lhs)) {
						error(codes::asl("not-borrowable"), "'&' needs a variable, a field or an element");

					} else if (writes) {
						if (const auto denied = readOnly(lhs, true); denied.has_value()) {
							deniedBorrow(lhs, *denied);
						}
					}

					_mod.stmts[ref_.index].type = type;
					size_t targetIndex = ~0uz;
					size_t parentAlias = ~0uz;
					if (const auto* root = rootOf(lhs); root != nullptr && root->kind == ExprKind::eNameRef) {
						if (const auto* rootEntry = lookup(*root); rootEntry != nullptr) {
							if (not rootEntry->target.empty()) {
								parentAlias = static_cast<size_t>(rootEntry - _scope.data());
							}

							if (rootEntry->targetIndex != ~0uz) {
								targetIndex = rootEntry->targetIndex;

							} else if (not rootEntry->target.empty()) {
								targetIndex = ~0uz;

							} else {
								targetIndex = static_cast<size_t>(rootEntry - _scope.data());
							}
						}
					}

					auto place = placePath(lhs);
					if (const auto* live = place.has_value() ? liveConflict(*place, parentAlias, writes) : nullptr; live != nullptr) {
						error(
							codes::asl("borrow-conflict"),
							"'" + String { writes ? "&mut" : "&" } + "' borrow '" + ir::describePlace(_mod, *place) +
							"' overlaps live borrow '" +
							ir::describePlace(_mod, *live->place) + "'"
						);
					}

					_scope.emplace_back(
						_mod.stmts[ref_.index].name,
						type,
						writes ? Mode::eRefWrite : Mode::eRefRead,
						rootName(lhs),
						targetIndex,
						std::move(place),
						parentAlias
					);
					break;
				}
				case StmtKind::eAssign: {
					const auto op = _mod.stmts[ref_.index].op;
					TypeRef target;
					TypeRef value;
					if (op == ir::Op::eNone) {
						target = expr(lhs);
						value = expr(rhs);

					} else {
						// `a op= b` checks as `a op b` that must land back in `a`
						target = expr(lhs);
						_compound = lhs.index;
						value = binary(op, Vector<ExprRef> { lhs, rhs });
						_compound = ~0uz;
						if (valid(target) && valid(value) && not same(target, value)) {
							error(
								codes::asl("type-mismatch"),
								"cannot assign " + ir::quotedType(_mod.types, value) + " to " + ir::quotedType(_mod.types, target)
							);
						}
					}

					if (lhs.index < _mod.exprs.size() && ir::isPlace(_mod, lhs)) {
						if (const auto denied = readOnly(lhs, false); denied.has_value()) {
							error(codes::asl("not-assignable"), *denied);
						}
					}

					if (op == ir::Op::eNone && valid(target) && valid(value) && not coerce(rhs, target)) {
						incompatible(
							"cannot assign " + ir::quotedType(_mod.types, value) + " to " + ir::quotedType(_mod.types, target),
							rhs,
							value,
							target
						);
					}
					break;
				}
				case StmtKind::eWhile:
				case StmtKind::eFor: {
					const auto mark = _scope.size();
					if (kind == StmtKind::eFor && not children.empty()) {
						stmt(children[0]);
					}

					loopCondition(lhs);
					++_loops;
					if (children.size() > (kind == StmtKind::eFor ? 1uz : 0uz)) {
						stmt(kind == StmtKind::eFor ? children[1] : children[0]);
					}

					--_loops;
					if (kind == StmtKind::eFor && children.size() > 2uz) {
						stmt(children[2]);
					}

					_scope.resize(mark);
					break;
				}
				case StmtKind::eBreak:
				case StmtKind::eContinue: {
					if (_loops == 0uL) {
						error(
							codes::asl("loop-control-outside-loop"),
							String(kind == StmtKind::eBreak ? "'break'" : "'continue'") + " outside a loop"
						);
					}
					break;
				}
				case StmtKind::eDiscard: {
					if (_mod.entry != ir::EntryKind::eFragment) {
						error(codes::asl("stage-unavailable"), "'discard' is only valid in fragment stages");
					}
					break;
				}
				case StmtKind::eExpr: {
					if (lhs.index < _mod.exprs.size()) {
						(void)expr(lhs);
					}
					break;
				}
				case StmtKind::eReturn: {
					if (lhs.index < _mod.exprs.size()) {
						const auto value = expr(lhs);
						if (valid(_ret) && valid(value) && not coerce(lhs, _ret)) {
							incompatible(
								"cannot return " + ir::quotedType(_mod.types, value) + " from a function returning " + ir::quotedType(
									_mod.types,
									_ret
								),
								lhs,
								value,
								_ret
							);
						}
					}
					break;
				}
				case StmtKind::eIf: {
					const auto condition = expr(lhs);
					if (valid(condition) && not isBool(condition)) {
						error(
							codes::asl("type-mismatch"),
							"condition has type " + ir::quotedType(_mod.types, condition) + ", expected 'bool'"
						);
					}
					scoped(children);
					break;
				}
				case StmtKind::eBlock:
				case StmtKind::eWhen:
				default: {
					scoped(children);
					break;
				}
			}
		}

		[[nodiscard]] static bool constantTrue(ref<const ir::LiteralValue> literal_) {
			const auto* value = std::get_if<bool>(&literal_.value);
			return value != nullptr && *value;
		}

		/* D8: a constant-true or missing condition keeps the whole wave in the loop */
		void loopCondition(ExprRef cond_) {
			if (cond_.index >= _mod.exprs.size() || (_mod.exprs[cond_.index].kind == ExprKind::eLiteral && constantTrue(_mod.exprs[cond_.index].literal))) {
				_diags.emplace_back(
					ir::Diagnostic::error(codes::asl("constant-loop-condition"), "loop condition is always true").at(spanIn(_mod, _loc))
					.note("invocations that never reach 'break' keep the others waiting in the loop")
					.note("use a condition that bounds the loop")
				);
				return;
			}

			const auto condition = expr(cond_);
			if (valid(condition) && not isBool(condition)) {
				error(codes::asl("type-mismatch"), "condition has type " + ir::quotedType(_mod.types, condition) + ", expected 'bool'");
			}
		}

		/* Locals declared inside do not leak out */
		void scoped(ref<const Vector<ir::StmtRef>> children_) {
			const auto mark = _scope.size();
			for (const auto child : children_) {
				stmt(child);
			}
			_scope.resize(mark);
		}

		/* Expressions */

		[[nodiscard]] TypeRef expr(ExprRef ref_) {
			if (ref_.index >= _mod.exprs.size() || _depth > maxDepth) {
				return {};
			}

			const Nest nest { _depth };
			const Here here { _loc, _mod.exprs[ref_.index].loc };
			const auto result = exprImpl(ref_);
			_mod.exprs[ref_.index].type = result;
			return result;
		}

		[[nodiscard]] TypeRef exprImpl(ExprRef ref_) {
			const auto kind = _mod.exprs[ref_.index].kind;
			const auto text = _mod.exprs[ref_.index].name;
			const auto operands = _mod.exprs[ref_.index].operands;

			switch (kind) {
				case ExprKind::eLiteral: {
					const auto& value = _mod.exprs[ref_.index].literal.value;
					if (std::holds_alternative<bool>(value)) {
						return boolType();
					}

					const auto scalar = std::holds_alternative<f64>(value) ? ScalarKind::eFloat : std::holds_alternative<u64>(value) ? ScalarKind::eUInt : ScalarKind::eInt;
					return scalarOf(scalar, 32uL);
				}
				case ExprKind::eNameRef: return nameRef(ref_);
				case ExprKind::eBuiltin: return text == "bitcast" ? bitcast(ref_, operands) : builtin(text, operands);
				case ExprKind::eCall: return call(ref_, text, operands);
				case ExprKind::eField: return field(text, operands);
				case ExprKind::eIndex: return index(operands);
				case ExprKind::eRange: return range(ref_, operands);
				case ExprKind::eUnary: return unary(_mod.exprs[ref_.index].op, operands);
				case ExprKind::eBinary: return binary(_mod.exprs[ref_.index].op, operands);
				case ExprKind::eTernary: return ternary(operands);
				case ExprKind::eConstruct: {
					Vector<TypeRef> types {};
					for (const auto operand : operands) {
						types.emplace_back(expr(operand));
					}

					const auto type = _mod.exprs[ref_.index].type;
					if (valid(type) && _mod.types[type.index].kind == TypeKind::eArray) {
						const auto count = _mod.types[type.index].count;
						const auto element = _mod.types[type.index].element;
						if (operands.size() != count) {
							error(
								codes::asl("wrong-argument-count"),
								"array of " + std::to_string(count) + " elements constructed from " + std::to_string(operands.size()) +
								" values"
							);
						}

						for (auto i = 0uz; i < operands.size(); ++i) {
							if (valid(types[i]) && not coerce(operands[i], element)) {
								error(
									codes::asl("type-mismatch"),
									"array element " + std::to_string(i) + " is " + ir::quotedType(_mod.types, types[i]) + ", expected " +
									ir::quotedType(_mod.types, element)
								);
							}
						}
					}

					if (valid(type) && _mod.types[type.index].kind == TypeKind::eMatrix) {
						const auto desc = _mod.types[type.index];
						const auto column = vectorOf(desc.scalar, desc.bits, desc.m);
						const auto fromMatrix = operands.size() == 1uz && valid(types[0]) && _mod.types[types[0].index].kind ==
							TypeKind::eMatrix &&
							_mod.types[types[0].index].scalar == desc.scalar && _mod.types[types[0].index].n >= desc.n &&
							_mod.types[types[0].index].m >= desc.m;
						auto fromColumns = operands.size() == desc.n;
						for (auto i = 0uz; fromColumns && i < operands.size(); ++i) {
							fromColumns = valid(types[i]) && same(types[i], column);
						}

						/* one scalar (diagonal) or n*m scalars (column-major), all of the matrix scalar kind */
						auto fromScalars = not fromMatrix && not fromColumns && (operands.size() == 1uz || operands.size() == static_cast<
							size_t>(desc.n) * desc.m);
						const auto scalarType = scalarOf(desc.scalar, desc.bits);
						for (auto i = 0uz; fromScalars && i < operands.size(); ++i) {
							fromScalars = valid(types[i]) && coerce(operands[i], scalarType);
						}

						if (not fromMatrix && not fromColumns && not fromScalars) {
							error(
								codes::asl("type-mismatch"),
								"cannot construct " + ir::quotedType(_mod.types, type) + " from the given arguments"
							);
						}
					}

					if (isKind(type, TypeKind::eScalar) || isKind(type, TypeKind::eVector)) {
						construct(type, operands, types);
					}
					return type;
				}
				default: return {};
			}
		}

		[[nodiscard]] const Entry* lookup(ref<const ir::Expr> e_) const {
			for (auto it = _scope.rbegin(); it != _scope.rend(); ++it) {
				if (it->name == e_.name && (e_.local || (e_.binding.index >= _mod.bindings.size() && e_.constant.index >= _mod.constants.
					size()))) {
					return &*it;
				}
			}
			return nullptr;
		}

		/* root node of a field/index chain; null when the chain breaks */
		[[nodiscard]] const ir::Expr* rootOf(ExprRef ref_) const {
			const ir::Expr* e = ref_.index < _mod.exprs.size() ? &_mod.exprs[ref_.index] : nullptr;
			while (e != nullptr && (e->kind == ExprKind::eField || e->kind == ExprKind::eIndex || e->kind == ExprKind::eRange)) {
				e = not e->operands.empty() && e->operands.front().index < _mod.exprs.size() ?
					&_mod.exprs[e->operands.front().index] :
					nullptr;
			}
			return e;
		}

		/* reports the `&mut` borrow `readOnly` denied; a `const` local root of the root module gets the fix that declares it with `let` */
		void deniedBorrow(ExprRef place_, String message_) {
			auto diagnostic = ir::Diagnostic::error(codes::asl("not-assignable"), std::move(message_)).at(spanIn(_mod, _loc));
			const auto* root = rootOf(place_);
			const auto* entry = root != nullptr && root->kind == ExprKind::eNameRef ? lookup(*root) : nullptr;
			if (entry != nullptr && entry->mode == Mode::eConstant && entry->keyword.length != 0uL) {
				std::move(diagnostic).fix(
					engine::diag::Fix {
						"declare '" + entry->name + "' with 'let'", { engine::diag::TextEdit { spanIn(_mod, entry->keyword), "let" } },
						engine::diag::Applicability::eMachineApplicable
					}
				);
			}

			_diags.emplace_back(std::move(diagnostic));
		}

		/* Why the place cannot be written (assignment) or borrowed `&mut`; none when it can */
		[[nodiscard]] Opt<String> readOnly(ExprRef place_, bool borrow_) const {
			const auto* root = rootOf(place_);
			/* tess_ctrl: a per-vertex output is written at gl_InvocationID only */
			if (_mod.entry == ir::EntryKind::eTessCtrl && root != nullptr) {
				const auto* var = root->kind == ExprKind::eBuiltin ? ir::findBuiltinVariable(root->name) : nullptr;
				const auto bindingIt = root->kind == ExprKind::eNameRef && root->binding.index < _mod.bindings.size() ?
					_mod.bindings.begin() + root->binding.index :
					_mod.bindings.end();
				const auto perVertex = (var != nullptr && var->arrayed && var->writable) ||
					(bindingIt != _mod.bindings.end() && bindingIt->arrayed && not bindingIt->input);

				/* the index applied to the root itself selects the vertex, however deep the place goes */
				const ir::Expr* vertexIndex = nullptr;
				for (const auto* e = &_mod.exprs[place_.index]; e != root; e = &_mod.exprs[e->operands.front().index]) {
					vertexIndex = e->kind == ExprKind::eIndex ? e : vertexIndex;
				}

				const auto indexRef = vertexIndex != nullptr && vertexIndex->operands.size() == 2uz ?
					vertexIndex->operands[1uz] :
					ExprRef {};
				const auto atInvocation = indexRef.index < _mod.exprs.size() && _mod.exprs[indexRef.index].kind == ExprKind::eBuiltin &&
					_mod.exprs[indexRef.index].name == "invocation_id";

				if (perVertex && not atInvocation) {
					return SomeVal<String>("per-vertex output '" + root->name + "' must be written at invocation_id");
				}
			}

			if (root != nullptr && root->kind == ExprKind::eBuiltin) {
				const auto* variable = ir::findBuiltinVariable(root->name);
				if (variable != nullptr && (not variable->writable || (variable->writeStages & ir::stageBit(_mod.entry)) == 0u)) {
					return SomeVal<String>("cannot assign to read-only builtin '" + root->name + "'");
				}

				return None;
			}

			if (root == nullptr || root->kind != ExprKind::eNameRef) {
				return None;
			}

			if (root->constant.index < _mod.constants.size()) {
				return SomeVal<String>("cannot assign to constant '" + root->name + "'");
			}

			if (const auto* entry = lookup(*root); entry != nullptr) {
				if (entry->mode == Mode::eMutable || entry->mode == Mode::eRefWrite) {
					return None;
				}

				if (borrow_) {
					return SomeVal<String>("cannot borrow immutable '" + root->name + "' as mutable");
				}

				return SomeVal<String>(
					entry->mode == Mode::eConstant ?
					"cannot assign to constant '" + root->name + "'" :
					"cannot assign through immutable reference '" + root->name + "'"
				);
			}

			const auto it = root->binding.index < _mod.bindings.size() ?
				_mod.bindings.begin() + root->binding.index :
				std::ranges::find(_mod.bindings, root->name, &ir::BindingDecl::localName);
			if (it == _mod.bindings.end() || it->bindClass == ir::BindClass::eWorkgroup || not it->input || it->access !=
				ir::Access::eRead) {
				return None;
			}

			return SomeVal<String>(
				borrow_ ?
				"cannot borrow read-only binding '" + root->name + "' as mutable" :
				"cannot assign to read-only binding '" + root->name + "'"
			);
		}

		/* root name of a place with `&mut` aliases resolved to the place they borrow */
		[[nodiscard]] String rootName(ExprRef ref_) const {
			const auto* root = rootOf(ref_);
			if (root == nullptr || root->kind != ExprKind::eNameRef) {
				return {};
			}

			const auto* entry = lookup(*root);
			return entry != nullptr && not entry->target.empty() ? entry->target : root->name;
		}

		/* what each name in a bound resolves to right now (DFS order), so a shadowing name never compares equal to the one it hides */
		void nameIds(ExprRef ref_, Vector<size_t>& out_, u32 depth_ = 0uL) const {
			if (ref_.index >= _mod.exprs.size() || depth_ > maxDepth) {
				out_.emplace_back(ir::unknownIdentity);
				return;
			}

			const auto& e = _mod.exprs[ref_.index];
			if (e.kind == ExprKind::eNameRef || e.kind == ExprKind::eBuiltin) {
				const auto* entry = e.kind == ExprKind::eNameRef ? lookup(e) : nullptr;
				out_.emplace_back(
					not immutableName(e) ?
					ir::unknownIdentity :
					entry != nullptr ?
					static_cast<size_t>(entry - _scope.data()) :
					ir::globalIdentity
				);
			}

			for (const auto o : e.operands) {
				nameIds(o, out_, depth_ + 1uL);
			}
		}

		/* the place an expression names, aliases expanded to what they borrow; none when it is no place */
		[[nodiscard]] Opt<ir::PlacePath> placePath(ExprRef ref_) const {
			Vector<const ir::Expr*> chain {};
			const ir::Expr* e = ref_.index < _mod.exprs.size() ? &_mod.exprs[ref_.index] : nullptr;
			while (e != nullptr && (e->kind == ExprKind::eField || e->kind == ExprKind::eIndex || e->kind == ExprKind::eRange)) {
				chain.emplace_back(e);
				e = not e->operands.empty() && e->operands.front().index < _mod.exprs.size() ?
					&_mod.exprs[e->operands.front().index] :
					nullptr;
			}

			if (e == nullptr || (e->kind != ExprKind::eNameRef && e->kind != ExprKind::eBuiltin)) {
				return None;
			}

			ir::PlacePath path { .root = e->name };
			if (e->kind == ExprKind::eNameRef) {
				if (const auto* entry = lookup(*e); entry != nullptr && not entry->target.empty()) {
					if (entry->place.has_value()) {
						path = *entry->place;

					} else {
						path.root = entry->target;
						path.rootScope = entry->targetIndex;
					}

				} else if (entry != nullptr) {
					path.rootScope = static_cast<size_t>(entry - _scope.data());
				}
			}

			for (auto it = chain.rbegin(); it != chain.rend(); ++it) {
				const auto& ops = (*it)->operands;
				if ((*it)->kind == ExprKind::eField) {
					path.steps.emplace_back(ir::PlaceStep { .kind = ir::PlaceStep::Kind::eField, .field = (*it)->name });

				} else if ((*it)->kind == ExprKind::eIndex && ops.size() >= 2uz) {
					ir::PlaceStep step { .kind = ir::PlaceStep::Kind::eIndex, .index = ops[1] };
					nameIds(ops[1], step.indexIds);
					path.steps.emplace_back(std::move(step));

				} else if ((*it)->kind == ExprKind::eRange && ops.size() >= 3uz) {
					ir::PlaceStep step { .kind = ir::PlaceStep::Kind::eRange, .lo = ops[1], .hi = ops[2] };
					nameIds(ops[1], step.loIds);
					nameIds(ops[2], step.hiIds);
					path.steps.emplace_back(std::move(step));

				} else {
					return None;
				}
			}

			return SomeVal(::hg::move(path));
		}

		/* maximal places inside an expression (index and range bounds included) */
		void collectPlaces(const ExprRef ref_, Vector<ir::PlacePath>& out_, const u32 depth_ = 0uL) const {
			if (ref_.index >= _mod.exprs.size() || depth_ > maxDepth) {
				return;
			}

			if (auto path = placePath(ref_); path.has_value()) {
				out_.emplace_back(std::move(*path));
				for (const auto* e = &_mod.exprs[ref_.index]; e->kind != ExprKind::eNameRef && e->kind != ExprKind::eBuiltin;
				     e = &_mod.exprs[e->operands.front().index]) {
					for (auto k = 1uz; k < e->operands.size(); ++k) {
						collectPlaces(e->operands[k], out_, depth_ + 1uL);
					}
				}
				return;
			}

			for (const auto o : _mod.exprs[ref_.index].operands) {
				collectPlaces(o, out_, depth_ + 1uL);
			}
		}

		/* names a bound may mention: constants, by-value parameters and locals, read-only builtins; by-value params count because they are entered as Mode::eConstant */
		[[nodiscard]] bool immutableName(ref<const ir::Expr> e_) const {
			if (e_.kind == ExprKind::eBuiltin) {
				const auto* variable = ir::findBuiltinVariable(e_.name);
				return variable != nullptr && not variable->writable;
			}

			if (const auto* entry = lookup(e_); entry != nullptr) {
				return entry->mode == Mode::eConstant;
			}

			return e_.constant.index < _mod.constants.size();
		}

		/* first live `let` borrow overlapping `place_` (two reads never conflict); aliases `from_` and its ancestors are exempt, as the place derives from them */
		[[nodiscard]] const Entry* liveConflict(ref<const ir::PlacePath> place_, const size_t from_, const bool writes_) const {
			const auto immutable = [this](ref<const ir::Expr> e_) { return immutableName(e_); };
			for (auto i = 0uz; i < _scope.size(); ++i) {
				const auto& live = _scope[i];
				if (not live.place.has_value() || (not writes_ && live.mode != Mode::eRefWrite)) {
					continue;
				}

				auto ancestor = false;
				for (auto p = from_; p != ~0uz && p < _scope.size(); p = _scope[p].parentAlias) {
					ancestor = ancestor || p == i;
				}

				if (not ancestor && not ir::disjoint(_mod, place_, *live.place, immutable)) {
					return &live;
				}
			}

			return nullptr;
		}

		/* a borrowing argument against the live `let` borrows; none fires for an argument rooted in an alias it derives from */
		void liveArgument(ref<const String> name_, const ExprRef arg_, const bool writes_) {
			const auto mine = placePath(arg_);
			if (not mine.has_value()) {
				return;
			}

			auto from = ~0uz;
			if (const auto* root = rootOf(arg_); root != nullptr && root->kind == ExprKind::eNameRef) {
				if (const auto* entry = lookup(*root); entry != nullptr && not entry->target.empty()) {
					from = static_cast<size_t>(entry - _scope.data());
				}
			}

			if (const auto* live = liveConflict(*mine, from, writes_); live != nullptr) {
				error(
					codes::asl("borrow-conflict"),
					"'" + String { writes_ ? "&mut" : "&" } + "' argument '" + ir::describePlace(_mod, *mine) + "' overlaps live borrow '" +
					ir::describePlace(_mod, *live->place) + "' of call to '" + name_ + "'"
				);
			}
		}

		/* the `&mut` argument `index_` must not overlap any place in the other arguments; true when it reported */
		bool overlap(ref<const String> name_, ref<const Vector<ExprRef>> operands_, const size_t index_) {
			const auto mine = placePath(operands_[index_]);
			if (not mine.has_value()) {
				return false;
			}

			const auto immutable = [this](ref<const ir::Expr> e_) { return immutableName(e_); };
			for (auto j = 0uz; j < operands_.size(); ++j) {
				Vector<ir::PlacePath> others {};
				if (j != index_) {
					collectPlaces(operands_[j], others);
				}

				for (const auto& other : others) {
					if (not ir::disjoint(_mod, *mine, other, immutable)) {
						_diags.emplace_back(
							ir::Diagnostic::error(
								codes::asl("borrow-conflict"),
								"'&mut' argument '" + ir::describePlace(_mod, *mine) + "' overlaps argument '" + ir::describePlace(
									_mod,
									other
								) +
								"' of call to '" + name_ + "'"
							)
							.at(spanIn(_mod, _mod.exprs[operands_[index_].index].loc))
							.label(spanIn(_mod, _mod.exprs[operands_[j].index].loc), "overlaps the '&mut' argument")
						);
						return true;
					}
				}
			}

			return false;
		}

		/* `&mut` parameters: the argument must be a writable place, sharing no root with another argument; `&` and `&mut` both respect live borrows */
		void refArguments(ref<const String> name_, ref<const ir::FunctionDecl> fn_, ref<const Vector<ExprRef>> operands_) {
			for (auto i = 0uz; i < fn_.params.size() && i < operands_.size(); ++i) {
				if (fn_.params[i].kind == ir::ParamKind::eRef) {
					liveArgument(name_, operands_[i], false);
				}

				if (fn_.params[i].kind != ir::ParamKind::eRefMut) {
					continue;
				}

				if (not ir::isPlace(_mod, operands_[i])) {
					error(codes::asl("not-borrowable"), "'&' needs a variable, a field or an element");
					continue;
				}

				if (const auto denied = readOnly(operands_[i], true); denied.has_value()) {
					deniedBorrow(operands_[i], *denied);
					continue;
				}

				if (not overlap(name_, operands_, i)) {
					liveArgument(name_, operands_[i], true);
				}
			}
		}

		/* module constants: a declared type, a constant initializer; a spec constant is a scalar */
		void constant(const size_t index_) {
			const auto& decl = _mod.constants[index_];
			const Here here { _loc, decl.loc };
			if (not valid(decl.type)) {
				return;
			}

			const auto init = expr(decl.value);
			if (valid(init) && not coerce(decl.value, decl.type)) {
				incompatible(
					"cannot initialize constant '" + decl.name + "' of type " + ir::quotedType(_mod.types, decl.type) + " with " +
					ir::quotedType(_mod.types, init),
					decl.value,
					init,
					decl.type
				);
			}

			if (decl.specId.has_value() && _mod.types[decl.type.index].kind != TypeKind::eScalar) {
				error(
					codes::asl("type-mismatch"),
					"specialization constant '" + decl.name + "' must be a scalar, not " + ir::quotedType(_mod.types, decl.type)
				);
			}

			if (not isConstant(decl.value)) {
				error(codes::asl("not-constant"), "initializer of constant '" + decl.name + "' is not a constant expression");
			}
		}

		[[nodiscard]] bool isConstant(const ExprRef ref_, const u32 depth_ = 0uL) const {
			if (ref_.index >= _mod.exprs.size() || depth_ > maxDepth) {
				return false;
			}

			const auto& e = _mod.exprs[ref_.index];
			switch (e.kind) {
				case ExprKind::eLiteral: return true;
				case ExprKind::eNameRef: return e.constant.index < _mod.constants.size();
				case ExprKind::eConstruct:
				case ExprKind::eUnary:
				case ExprKind::eBinary:
				case ExprKind::eTernary:
				case ExprKind::eField:
				case ExprKind::eIndex: {
					return std::ranges::all_of(e.operands, [&](ExprRef o_) { return isConstant(o_, depth_ + 1uL); });
				}
				default: return false;
			}
		}

		[[nodiscard]] TypeRef nameRef(const ExprRef ref_) {
			const auto& e = _mod.exprs[ref_.index];
			if (e.constant.index < _mod.constants.size()) {
				return _mod.constants[e.constant.index].type;
			}

			if (const auto* entry = lookup(e); entry != nullptr) {
				return entry->type;
			}

			/* a per-vertex input is read as an unsized array of its declared element type */
			const auto typeOf = [this](ref<const ir::BindingDecl> decl_) {
				if (not decl_.arrayed) {
					return decl_.type;
				}
				ir::TypeDesc desc {};
				desc.kind = ir::TypeKind::eArray;
				desc.element = decl_.type;
				return intern(std::move(desc));
			};

			if (e.binding.index < _mod.bindings.size()) {
				return typeOf(_mod.bindings[e.binding.index]);
			}

			const auto it = std::ranges::find(_mod.bindings, e.name, &ir::BindingDecl::localName);
			if (it == _mod.bindings.end()) {
				error(codes::asl("unknown-name"), "unknown name '" + e.name + "'");
				return {};
			}

			return typeOf(*it);
		}

		[[nodiscard]] TypeRef builtin(ref<const String> name_, ref<const Vector<ExprRef>> operands_) {
			if (const auto* variable = ir::findBuiltinVariable(name_); variable != nullptr && operands_.empty()) {
				if (_hasMain && (variable->stages & ir::stageBit(_mod.entry)) == 0u) {
					error(
						codes::asl("stage-unavailable"),
						"'" + name_ + "' is not available in " + String(ir::stageName(_mod.entry)) + " stages"
					);
				}

				const auto element = vectorOf(variable->scalar, 32uL, variable->n);
				if (variable->arrayed || variable->count != 0uL) {
					ir::TypeDesc desc {};
					desc.kind = ir::TypeKind::eArray;
					desc.element = element;
					desc.count = variable->count;
					return intern(std::move(desc));
				}

				return element;
			}

			if (name_ == "len") {
				// region length; not a GLSL builtin, lowered with the regions
				if (operands_.size() != 1uz) {
					error(codes::asl("wrong-argument-count"), "'len' takes one region argument");
					return {};
				}

				const auto arg = expr(operands_.front());
				if (valid(arg) && not isKind(arg, TypeKind::eRegion)) {
					error(codes::asl("type-mismatch"), "'len' needs a region, got " + ir::quotedType(_mod.types, arg));
					return {};
				}

				return valid(arg) ? scalarOf(ScalarKind::eUInt, 32uL) : TypeRef {};
			}

			Vector<TypeRef> types {};
			for (const auto operand : operands_) {
				types.emplace_back(expr(operand));
			}

			const auto* first = ir::findBuiltinFunction(name_);
			if (first == nullptr) {
				return {};
			}

			if (_hasMain && (first->stages & ir::stageBit(_mod.entry)) == 0u) {
				error(
					codes::asl("stage-unavailable"),
					"'" + name_ + "' is not available in " + String(ir::stageName(_mod.entry)) + " stages"
				);
				return {};
			}

			for (const auto type : types) {
				if (not valid(type)) {
					return {};
				}
			}

			const auto sized = not types.empty() && (isKind(types.front(), TypeKind::eTexture) || isKind(types.front(), TypeKind::eImage));
			const auto dim = sized ? _mod.types[types.front().index].dim : ir::TextureDim::e2D;
			const StringView format = sized && isKind(types.front(), TypeKind::eImage) ?
				StringView { _mod.types[types.front().index].name } :
				StringView {};

			const auto& signatures = ir::parsedBuiltinSignatures();
			/* all-integer arguments try the genI/genU overloads before genF, so `max(1, 2)` is s32 (D4) */
			auto allInt = not types.empty();
			for (const auto type : types) {
				const auto& d = _mod.types[type.index];
				allInt = allInt && (d.kind == TypeKind::eScalar || d.kind == TypeKind::eVector) &&
					(d.scalar == ScalarKind::eInt || d.scalar == ScalarKind::eUInt);
			}

			for (auto pass = allInt ? 0 : 1; pass < 2; ++pass)
				for (auto i = 0uz; i < std::size(ir::builtinFunctions); ++i) {
					const auto& fn = ir::builtinFunctions[i];
					const auto& sig = signatures[i];
					if (fn.name != name_ || not sig.valid || sig.count != types.size()) {
						continue;
					}
					if (pass == 0 && sig.params[0].family == ir::BuiltinFamily::eGenF) {
						continue;
					}

					for (auto width = 1uL; width <= 4uL; ++width) {
						Vector<TypeRef> targets {};
						auto ok = true;
						for (auto k = 0uz; k < types.size() && ok; ++k) {
							const auto desc = ir::resolveBuiltinSigType(sig.params[k], width, dim, format);
							ok = desc.has_value();
							if (ok) {
								targets.emplace_back(intern(*desc));
								ok = same(types[k], targets.back()) || adaptable(operands_[k], targets.back());
							}
						}

						const auto ret = ir::resolveBuiltinSigType(sig.ret, width, dim, format);
						if (not ok || not ret.has_value()) {
							continue;
						}

						for (auto k = 0uz; k < types.size(); ++k) {
							(void)coerce(operands_[k], targets[k]);
						}

						for (auto k = 0uz; k < types.size(); ++k) {
							if (not sig.params[k].mut) {
								continue;
							}

							if (not ir::isPlace(_mod, operands_[k])) {
								error(codes::asl("not-borrowable"), "'&' needs a variable, a field or an element");
								continue;
							}

							if (const auto denied = readOnly(operands_[k], true); denied.has_value()) {
								deniedBorrow(operands_[k], *denied);
								continue;
							}

							const auto isImage = name_ == "image_store";
							if (const auto* rootExpr = rootOf(operands_[k]);
								(isImage || name_.starts_with("atomic_")) && rootExpr != nullptr && rootExpr->kind == ExprKind::eNameRef) {
								const auto* entry = lookup(*rootExpr);
								/* an alias borrows `target`; resolve it to the scope entry or binding it names */
								const auto& rootLabel = entry != nullptr && not entry->target.empty() ? entry->target : rootExpr->name;
								const auto* viaScope = entry;

								if (entry != nullptr && not entry->target.empty()) {
									/* by scope index, not name: a shadowing local must not capture the alias */
									viaScope = entry->targetIndex < _scope.size() ? &_scope[entry->targetIndex] : nullptr;
								}

								const ir::BindingDecl* bound = nullptr;
								if (viaScope == nullptr) {
									if (entry == nullptr && rootExpr->binding.index < _mod.bindings.size()) {
										bound = &_mod.bindings[rootExpr->binding.index];

									} else if (const auto found = std::ranges::find(_mod.bindings, rootLabel, &ir::BindingDecl::localName);
										found != _mod.bindings.end()) {
										bound = &*found;
									}
								}

								const auto cls = bound != nullptr ? bound->bindClass : ir::BindClass::eNone;
								const auto good = isImage ?
									cls == ir::BindClass::eStorageImage :
									(cls == ir::BindClass::eStorageBuffer || cls == ir::BindClass::eWorkgroup);
								if (viaScope != nullptr && viaScope->mode == Mode::eRefWrite && viaScope->target.empty()) {
									String note = "GLSL ";
									note += isImage ? "image access needs" : "atomics need";
									note += " the ";
									note += isImage ? "storage image" : "storage buffer or workgroup variable";
									note += " itself";
									error(
										codes::asl("not-storage-resource"),
										"'" + name_ + "' cannot act through parameter '" + rootLabel + "'",
										std::move(note)
									);
									continue;
								}

								if (not good) {
									error(
										codes::asl("not-storage-resource"),
										"'" + name_ + "' needs a " + (isImage ? "storage image" : "storage buffer or workgroup") +
										" variable, got " +
										(bound != nullptr ? "binding '" : "local '") + rootLabel + "'"
									);
									continue;
								}
							}

							if (not overlap(name_, operands_, k)) {
								liveArgument(name_, operands_[k], true);
							}
						}

						return sig.ret.family == ir::BuiltinFamily::eVoid ? TypeRef {} : intern(*ret);
					}
				}

			String args {};
			for (auto k = 0uz; k < types.size(); ++k) {
				args += (k == 0uz ? "" : ", ") + ir::quotedType(_mod.types, types[k]);
			}

			error(codes::asl("no-matching-overload"), "no overload of '" + name_ + "' accepts (" + args + ")");
			return {};
		}

		[[nodiscard]] TypeRef field(ref<const String> name_, ref<const Vector<ExprRef>> operands_) {
			if (operands_.empty()) {
				return {};
			}

			const auto base = expr(operands_.front());
			if (not valid(base)) {
				return {};
			}

			const auto desc = _mod.types[base.index];
			if (desc.kind == TypeKind::eStruct) {
				const auto it = std::ranges::find(desc.fields, name_, &ir::FieldDesc::name);
				if (it != desc.fields.end()) {
					return it->type;
				}

			} else if (desc.kind == TypeKind::eVector || desc.kind == TypeKind::eScalar) {
				constexpr std::array<StringView, 3> sets { "xyzw", "rgba", "stpq" };
				for (const auto set : sets) {
					const auto fits = not name_.empty() && name_.size() <= 4uz && std::ranges::all_of(
						name_,
						[&](char c_) {
							const auto at = set.find(c_);
							return at != StringView::npos && at < desc.n;
						}
					);

					if (fits) {
						return vectorOf(desc.scalar, desc.bits, static_cast<u32>(name_.size()));
					}
				}
			}

			error(codes::asl("unknown-name"), "unknown field '" + name_ + "' of " + ir::quotedType(_mod.types, base));
			return {};
		}

		/* `base[lo .. hi]`: a region over an array or region; only legal under a borrow */
		[[nodiscard]] TypeRef range(ExprRef ref_, ref<const Vector<ExprRef>> operands_) {
			if (operands_.size() != 3uz) {
				return {};
			}

			if (std::ranges::find(_borrowed, ref_.index) == _borrowed.end()) {
				error(codes::asl("type-mismatch"), "a range needs a borrow");
			}

			const auto base = expr(operands_[0]);
			const auto u32Type = scalarOf(ScalarKind::eUInt, 32uL);
			auto bounds = true;
			for (const auto bound : { operands_[1], operands_[2] }) {
				const auto type = expr(bound);
				bounds = bounds && valid(type) && coerce(bound, u32Type);
			}

			if (not bounds) {
				error(codes::asl("type-mismatch"), "range bounds must be 'u32'");

			} else if (const auto& lo = _mod.exprs[operands_[1].index],& hi = _mod.exprs[operands_[2].index];
				lo.kind == ExprKind::eLiteral && hi.kind == ExprKind::eLiteral && ir::integerLiteral(lo) && ir::integerLiteral(hi)
				&&
				*ir::integerLiteral(lo) >= *ir::integerLiteral(hi)) {
				error(codes::asl("empty-range"), "empty or reversed range");
			}

			if (not valid(base)) {
				return {};
			}

			const auto desc = _mod.types[base.index];
			if (desc.kind != TypeKind::eArray && desc.kind != TypeKind::eRegion) {
				error(codes::asl("type-mismatch"), "cannot range " + ir::quotedType(_mod.types, base));
				return {};
			}

			TypeDesc region {};
			region.kind = TypeKind::eRegion;
			region.element = desc.element;
			return intern(std::move(region));
		}

		[[nodiscard]] TypeRef index(ref<const Vector<ExprRef>> operands_) {
			if (operands_.size() != 2uz) {
				return {};
			}

			const auto base = expr(operands_[0]);
			const auto at = expr(operands_[1]);
			if (valid(at) && not isInteger(at)) {
				error(codes::asl("type-mismatch"), "index has type " + ir::quotedType(_mod.types, at) + ", expected an integer");
			}

			if (not valid(base)) {
				return {};
			}

			const auto desc = _mod.types[base.index];
			switch (desc.kind) {
				case TypeKind::eArray: return desc.element;
				case TypeKind::eRegion: return desc.element;
				case TypeKind::eVector: return scalarOf(desc.scalar, desc.bits);
				case TypeKind::eMatrix: return vectorOf(desc.scalar, desc.bits, desc.m);
				default: break;
			}

			error(codes::asl("type-mismatch"), "cannot index type " + ir::quotedType(_mod.types, base));
			return {};
		}

		[[nodiscard]] TypeRef unary(ir::Op op_, ref<const Vector<ExprRef>> operands_) {
			if (operands_.empty()) {
				return {};
			}

			const auto inner = expr(operands_.front());
			if (not valid(inner)) {
				return {};
			}

			if (op_ == ir::Op::eNot ? isBool(inner) : (op_ == ir::Op::eBitNot ? isInteger(componentOf(inner)) : numeric(inner))) {
				return inner;
			}

			error(codes::asl("type-mismatch"), "operand of '" + String { ir::spelling(op_) } + "' has type " + ir::quotedType(_mod.types, inner));
			return {};
		}

		[[nodiscard]] TypeRef binary(ir::Op op_, ref<const Vector<ExprRef>> operands_) {
			if (operands_.size() != 2uz) {
				return {};
			}

			auto lt = expr(operands_[0]);
			auto rt = expr(operands_[1]);
			if (not valid(lt) || not valid(rt)) {
				return {};
			}

			if (op_ == ir::Op::eLogicalAnd || op_ == ir::Op::eLogicalOr) {
				return isBool(lt) && isBool(rt) ? boolType() : mismatch(ir::spelling(op_), lt, rt);
			}

			/* an untyped literal takes the component type of the other side */
			if (literalOf(operands_[0]) != Literal::eNone && literalOf(operands_[1]) == Literal::eNone &&
				adaptable(operands_[0], componentOf(rt))
			) {
				lt = componentOf(rt);
				retype(operands_[0], lt);

			} else if (literalOf(operands_[1]) != Literal::eNone && literalOf(operands_[0]) == Literal::eNone &&
				adaptable(operands_[1], componentOf(lt))
			) {
				rt = componentOf(lt);
				retype(operands_[1], rt);
			}

			if (op_ == ir::Op::eEq || op_ == ir::Op::eNe) {
				return same(lt, rt) ? boolType() : mismatched(op_, operands_, lt, rt);
			}

			if (op_ == ir::Op::eMod && numeric(lt) && numeric(rt) && not(isInteger(componentOf(lt)) && isInteger(componentOf(rt)))) {
				error(
					codes::asl("type-mismatch"),
					"operator '%' needs integer operands, got " + ir::quotedType(_mod.types, not isInteger(componentOf(lt)) ? lt : rt)
				);
				return {};
			}

			if (op_ == ir::Op::eShl || op_ == ir::Op::eShr) {
				return isInteger(componentOf(lt)) && isInteger(componentOf(rt)) ? lt : mismatch(ir::spelling(op_), lt, rt);
			}

			if (op_ == ir::Op::eBitAnd || op_ == ir::Op::eBitOr || op_ == ir::Op::eBitXor) {
				return isInteger(componentOf(lt)) && same(lt, rt) ? lt : mismatched(op_, operands_, lt, rt);
			}

			if (not numeric(lt) || not numeric(rt)) {
				return mismatch(ir::spelling(op_), lt, rt);
			}

			if (isComparison(op_)) {
				return isKind(lt, TypeKind::eScalar) && same(lt, rt) ? boolType() : mismatched(op_, operands_, lt, rt);
			}

			const auto l = _mod.types[lt.index];
			const auto r = _mod.types[rt.index];
			const auto comparable = l.scalar == r.scalar && l.bits == r.bits;
			if (comparable && op_ == ir::Op::eMul && l.kind == TypeKind::eMatrix && r.kind == TypeKind::eVector && l.n == r.n) {
				return vectorOf(l.scalar, l.bits, l.m);
			}

			if (comparable && op_ == ir::Op::eMul && l.kind == TypeKind::eVector && r.kind == TypeKind::eMatrix && r.m == l.n) {
				return vectorOf(r.scalar, r.bits, r.n);
			}

			if (comparable && op_ == ir::Op::eMul && l.kind == TypeKind::eMatrix && r.kind == TypeKind::eMatrix && l.n == r.m) {
				auto out = l;
				out.n = r.n;
				return intern(std::move(out));
			}

			if (same(lt, rt)) {
				return lt;
			}

			if (comparable && l.kind == TypeKind::eScalar) {
				return rt;
			}

			if (comparable && r.kind == TypeKind::eScalar) {
				return lt;
			}

			return mismatched(op_, operands_, lt, rt);
		}

		/* converts the rhs of a compound assignment, else the scalar side against a vector, else the narrower or non-float side, else the right side */
		[[nodiscard]] TypeRef mismatched(ir::Op op_, ref<const Vector<ExprRef>> operands_, TypeRef lt_, TypeRef rt_) {
			if (not numeric(lt_) || not numeric(rt_)) {
				return mismatch(ir::spelling(op_), lt_, rt_);
			}

			const auto& l = _mod.types[lt_.index];
			const auto& r = _mod.types[rt_.index];
			const auto right = operands_[0].index == _compound ? true :
				l.kind != TypeKind::eScalar && r.kind == TypeKind::eScalar ? true :
				l.kind == TypeKind::eScalar && r.kind != TypeKind::eScalar ? false :
				/* same family, different width: widen the narrower side */
				(l.scalar == ScalarKind::eFloat) == (r.scalar == ScalarKind::eFloat) && l.bits != r.bits ? r.bits < l.bits :
				l.scalar == ScalarKind::eFloat || r.scalar != ScalarKind::eFloat;
			const auto from = right ? rt_ : lt_;
			const auto other = right ? lt_ : rt_;
			const auto to = isKind(from, TypeKind::eScalar) ? componentOf(other) : other;
			incompatible(
				"operands of '" + String { ir::spelling(op_) } + "' have types " + ir::quotedType(_mod.types, lt_) + " and " +
				ir::quotedType(_mod.types, rt_),
				operands_[right ? 1uz : 0uz],
				from,
				to
			);
			return {};
		}

		/* `T(x)` converts (component-wise; a bool to 0/1; a number never to bool); several values build a vector of T's component */
		void construct(TypeRef type_, ref<const Vector<ExprRef>> operands_, ref<const Vector<TypeRef>> types_) {
			const auto target = _mod.types[type_.index];
			const auto component = scalarOf(target.scalar, target.bits);
			const auto width = [&](TypeRef t_) { return isKind(t_, TypeKind::eVector) ? _mod.types[t_.index].n : 1uL; };
			const auto plain = [&](TypeRef t_) { return isKind(t_, TypeKind::eScalar) || isKind(t_, TypeKind::eVector); };

			if (operands_.size() == 1uz) {
				const auto from = types_[0];
				if (not valid(from) || coerce(operands_[0], component) || coerce(operands_[0], type_)) {
					return;
				}

				if (not plain(from) || (width(from) != 1uL && width(from) != width(type_))) {
					error(
						codes::asl("type-mismatch"),
						"cannot construct " + ir::quotedType(_mod.types, type_) + " from " + ir::quotedType(_mod.types, from)
					);

				} else if (target.scalar == ScalarKind::eBool && _mod.types[from.index].scalar != ScalarKind::eBool) {
					error(
						codes::asl("type-mismatch"),
						"cannot convert " + ir::quotedType(_mod.types, from) + " to " + ir::quotedType(_mod.types, type_),
						"a number never converts to 'bool'; compare it with zero instead"
					);
				}
				return;
			}

			for (auto i = 0uz; i < operands_.size(); ++i) {
				if (not valid(types_[i])) {
					continue;
				}

				if (not plain(types_[i])) {
					error(
						codes::asl("type-mismatch"),
						"cannot construct " + ir::quotedType(_mod.types, type_) + " from " + ir::quotedType(_mod.types, types_[i])
					);
					continue;
				}

				const auto want = vectorOf(target.scalar, target.bits, width(types_[i]));
				if (not coerce(operands_[i], want)) {
					incompatible(
						"argument " + std::to_string(i) + " of " + ir::quotedType(_mod.types, type_) + " is " +
						ir::quotedType(_mod.types, types_[i]) + ", expected " + ir::quotedType(_mod.types, want),
						operands_[i],
						types_[i],
						want
					);
				}
			}
		}

		/* `bitcast<T>(x)` reinterprets a numeric scalar or vector as one of the same size */
		[[nodiscard]] TypeRef bitcast(ExprRef ref_, ref<const Vector<ExprRef>> operands_) {
			const auto target = _mod.exprs[ref_.index].type;
			const auto from = operands_.size() == 1uz ? expr(operands_[0]) : TypeRef {};
			if (not valid(from) || not valid(target)) {
				return {};
			}

			const auto plain = [&](TypeRef t_) { return numeric(t_) && not isKind(t_, TypeKind::eMatrix); };
			if (not plain(from) || not plain(target)) {
				error(
					codes::asl("type-mismatch"),
					"cannot bitcast " + ir::quotedType(_mod.types, from) + " to " + ir::quotedType(_mod.types, target) +
					", both need to be numeric scalars or vectors"
				);
				return {};
			}

			const auto bits = [&](TypeRef t_) {
				const auto& d = _mod.types[t_.index];
				return d.bits * (d.kind == TypeKind::eVector ? d.n : 1uL);
			};
			if (bits(from) != bits(target)) {
				error(
					codes::asl("bitcast-size"),
					"cannot bitcast " + ir::quotedType(_mod.types, from) + " (" + std::to_string(bits(from)) + " bits) to " +
					ir::quotedType(_mod.types, target) + " (" + std::to_string(bits(target)) + " bits)"
				);
				return {};
			}
			return target;
		}

		[[nodiscard]] TypeRef ternary(ref<const Vector<ExprRef>> operands_) {
			if (operands_.size() != 3uz) {
				return {};
			}

			const auto condition = expr(operands_[0]);
			auto thenType = expr(operands_[1]);
			const auto elseType = expr(operands_[2]);

			if (valid(condition) && not isBool(condition)) {
				error(codes::asl("type-mismatch"), "condition has type " + ir::quotedType(_mod.types, condition) + ", expected 'bool'");
			}

			if (not valid(thenType) || not valid(elseType) || same(thenType, elseType) || coerce(operands_[2], thenType)) {
				return thenType;
			}

			if (coerce(operands_[1], elseType)) {
				return elseType;
			}

			error(
				codes::asl("type-mismatch"),
				"branches of '?:' have types " + ir::quotedType(_mod.types, thenType) + " and " + ir::quotedType(_mod.types, elseType)
			);
			return {};
		}

		[[nodiscard]] TypeRef mismatch(StringView op_, TypeRef lt_, TypeRef rt_) {
			error(
				codes::asl("type-mismatch"),
				"operands of '" + String { op_ } + "' have types " + ir::quotedType(_mod.types, lt_) + " and " + ir::quotedType(_mod.types, rt_)
			);
			return {};
		}

		/* Calls: exact parameter types first, then with literal adaptation */

		/* an array argument binds to a region parameter over the same element type */
		[[nodiscard]] bool regionArgument(TypeRef arg_, TypeRef param_) const {
			return valid(arg_) && valid(param_) && _mod.types[arg_.index].kind == TypeKind::eArray &&
				_mod.types[param_.index].kind == TypeKind::eRegion && same(
					_mod.types[arg_.index].element,
					_mod.types[param_.index].element
				);
		}

		[[nodiscard]] TypeRef call(ExprRef ref_, ref<const String> name_, ref<const Vector<ExprRef>> operands_) {
			Vector<TypeRef> args {};
			auto known = true;
			for (const auto operand : operands_) {
				// a range argument is a borrow for a region parameter; other callees reject its region type
				if (_mod.exprs[operand.index].kind == ExprKind::eRange) {
					_borrowed.emplace_back(operand.index);
				}

				args.emplace_back(expr(operand));
				known = known && valid(args.back());
			}

			Vector<u32> named {};
			Vector<u32> arity {};
			for (auto i = 0uz; i < _mod.functions.size(); ++i) {
				if (_mod.functions[i].name == name_) {
					named.emplace_back(static_cast<u32>(i));
					if (_mod.functions[i].params.size() == args.size()) {
						arity.emplace_back(static_cast<u32>(i));
					}
				}
			}

			if (named.empty()) {
				error(codes::asl("unknown-function"), "unknown function '" + name_ + "'");
				return {};
			}

			const auto accepts = [&](u32 fn_, bool adapt_) {
				const auto& params = _mod.functions[fn_].params;
				for (auto i = 0uz; i < params.size(); ++i) {
					if (not same(args[i], params[i].type) && not regionArgument(args[i], params[i].type) &&
						not(adapt_ && adaptable(operands_[i], params[i].type))
					) {
						return false;
					}
				}
				return true;
			};

			Vector<u32> matches {};
			if (known) {
				for (const auto adapt : { false, true }) {
					for (const auto i : arity) {
						if (accepts(i, adapt)) {
							matches.emplace_back(i);
						}
					}

					if (not matches.empty()) {
						break;
					}
				}

			} else {
				matches = arity;
			}

			/* @when variants share one signature; pick the last one with a body, else the first */
			Opt<u32> chosen = None;
			String chosenSignature {};
			auto ambiguous = false;
			for (const auto i : matches) {
				auto signature = ir::signatureOf(_mod, _mod.functions[i]);
				if (not chosen.has_value()) {
					chosen = SomeVal(i);
					chosenSignature = std::move(signature);

				} else if (chosenSignature != signature) {
					ambiguous = true;

				} else if (_mod.functions[i].body.has_value()) {
					chosen = SomeVal(i);
				}
			}

			if (not chosen.has_value() || ambiguous) {
				if (known && ambiguous) {
					error(codes::asl("ambiguous-call"), "call to '" + name_ + "' is ambiguous for (" + join(args) + ")");

				} else if (known) {
					error(
						codes::asl("no-matching-overload"),
						"no overload of '" + name_ + "' takes (" + join(args) + ")",
						"candidates are " + candidates(named)
					);
				}
				return {};
			}

			const auto params = _mod.functions[*chosen].params;
			for (auto i = 0uz; known && i < params.size(); ++i) {
				(void)coerce(operands_[i], params[i].type);
			}

			for (auto i = 0uz; known && i < params.size(); ++i) {
				if (regionArgument(args[i], params[i].type) && _mod.types[args[i].index].count == 0uL) {
					error(codes::asl("type-mismatch"), "unsized array needs an explicit range");
				}
			}

			refArguments(name_, _mod.functions[*chosen], operands_);
			_mod.exprs[ref_.index].function = ir::FunctionRef { *chosen };
			return _mod.functions[*chosen].ret;
		}

		[[nodiscard]] String join(ref<const Vector<TypeRef>> types_) const {
			String text {};
			for (auto i = 0uz; i < types_.size(); ++i) {
				text += (i != 0uz ? ", " : "") + ir::quotedType(_mod.types, types_[i]);
			}
			return text;
		}

		[[nodiscard]] String candidates(ref<const Vector<u32>> named_) const {
			Vector<String> seen {};
			for (const auto i : named_) {
				auto signature = ir::signatureOf(_mod, _mod.functions[i]);
				if (std::ranges::find(seen, signature) == seen.end()) {
					seen.emplace_back(std::move(signature));
				}
			}

			String text {};
			for (auto i = 0uz; i < seen.size(); ++i) {
				text += (i != 0uz ? ", '" : "'") + seen[i] + "'";
			}

			return text;
		}
	};
}

String ir::describeType(ref<const Vector<TypeDesc>> types_, TypeRef type_, u32 depth_) {
	if (type_.index >= types_.size()) {
		return "<none>";
	}

	if (depth_ > describeDepthLimit) {
		return "...";
	}

	const auto& type = types_[type_.index];
	const auto scalar = type.scalar == ScalarKind::eBool ?
		String("bool") :
		(type.scalar == ScalarKind::eFloat ? "f" : (type.scalar == ScalarKind::eInt ? "s" : "u")) + std::to_string(type.bits);

	switch (type.kind) {
		case TypeKind::eScalar: return String(scalar);
		case TypeKind::eVector: return scalar + "vec" + std::to_string(type.n);
		case TypeKind::eMatrix: return scalar + "mat" + std::to_string(type.n) + (type.n == type.m ? "" : "x" + std::to_string(type.m));
		case TypeKind::eArray: {
			return describeType(types_, type.element, depth_ + 1uL) + "[" + (type.count == 0uL ? "" : std::to_string(type.count)) + "]";
		}
		case TypeKind::eStruct: {
			if (not type.name.empty()) {
				return type.name;
			}

			String text = "block { ";
			for (const auto& field : type.fields) {
				text += describeType(types_, field.type, depth_ + 1uL) + " " + field.name + "; ";
			}

			return text + "}";
		}
		case TypeKind::eTexture: {
			constexpr std::array<StringView, 5> dims { "1d", "2d", "3d", "cube", "2darray" };
			return "texture" + String(dims[std::min(static_cast<u32>(type.dim), u32 { 4 })]);
		}
		case TypeKind::eImage: {
			constexpr std::array<StringView, 5> dims { "1d", "2d", "3d", "cube", "2darray" };
			return "image" + String(dims[std::min(static_cast<u32>(type.dim), u32 { 4 })]) + "<" + type.name + ">";
		}
		case TypeKind::eRegion: return "&" + describeType(types_, type.element, depth_ + 1uL) + "[]";
		default: return "sampler";
	}
}

String ir::quotedType(ref<const Vector<TypeDesc>> types_, TypeRef type_) {
	return "'" + describeType(types_, type_) + "'";
}

String ir::signatureOf(ref<const Module> mod_, ref<const FunctionDecl> fn_) {
	String text = fn_.name + "(";
	for (auto i = 0uz; i < fn_.params.size(); ++i) {
		text += (i != 0uz ? ", " : "") + describeType(mod_.types, fn_.params[i].type);
	}

	return text + ")";
}

bool ir::isPlace(ref<const Module> mod_, ExprRef expr_) {
	const auto* e = expr_.index < mod_.exprs.size() ? &mod_.exprs[expr_.index] : nullptr;
	while (e != nullptr && (e->kind == ExprKind::eField || e->kind == ExprKind::eIndex || e->kind == ExprKind::eRange)) {
		e = not e->operands.empty() && e->operands.front().index < mod_.exprs.size() ? &mod_.exprs[e->operands.front().index] : nullptr;
	}

	return e != nullptr && (e->kind == ExprKind::eNameRef || (e->kind == ExprKind::eBuiltin && ir::findBuiltinVariable(e->name) !=
		nullptr));
}

ir::Diagnostics ir::checkStageValidity(ref<const Module> mod_, EntryKind entry_) {
	Diagnostics diags {};
	for (const auto& e : mod_.exprs) {
		if (e.kind != ExprKind::eBuiltin) {
			continue;
		}

		const auto* variable = findBuiltinVariable(e.name);
		const auto* function = variable == nullptr ? findBuiltinFunction(e.name) : nullptr;
		const auto stages = variable != nullptr ? variable->stages : (function != nullptr ? function->stages : stageAll);
		if ((stages & stageBit(entry_)) == 0u) {
			diags.emplace_back(
				Diagnostic::error(
					codes::asl("stage-unavailable"),
					"'" + e.name + "' is not available in " + String(stageName(entry_)) + " stages"
				)
				.at(spanIn(mod_, e.loc))
			);
		}
	}
	return diags;
}

namespace {
	/* D8: implicit-derivative builtins reached under lane-dependent control; warnings only, never failures */
	class DivergenceWarner {
	public:
		explicit DivergenceWarner(ref<const ir::Module> mod_) :
			_mod(mod_),
			_uniformity(
				ir::analyzeUniformity(
					mod_,
					[](StringView name_) {
						const auto* fn = ir::findBuiltinFunction(name_);
						return fn != nullptr && fn->needsUniformControl;
					}
				)
			),
			_warned(mod_.exprs.size(), false) {}

		[[nodiscard]] ir::Diagnostics run() {
			/* via-chain reports first (they win the per-call dedupe), then direct ones */
			for (const auto recurse : { true, false }) {
				for (auto f = 0uz; f < _mod.functions.size(); ++f) {
					visit(f, recurse, false);
				}
			}

			return std::move(_diags);
		}

	private:
		ref<const ir::Module> _mod;
		ir::Uniformity _uniformity;
		Vector<bool> _warned;
		ir::Diagnostics _diags;
		Vector<u32> _chain;
		bool _recurse = false;

		void warn(ir::ExprRef at_) {
			if (_warned[at_.index]) {
				return;
			}

			_warned[at_.index] = true;
			const auto& e = _mod.exprs[at_.index];
			const auto* fn = ir::findBuiltinFunction(e.name);
			const auto fatal = fn != nullptr && fn->errorUnderDivergence;
			String via {};
			for (auto it = _chain.rbegin(); it != _chain.rend(); ++it) {
				via += (via.empty() ? "" : ", ") + _mod.functions[*it].name;
			}

			const auto tail = via.empty() ? String {} : " (via " + via + ")";
			auto message = "'" + e.name + (fatal ?
					"' needs every invocation of a group together but runs" :
					"' picks its detail level automatically") +
				" under a condition that can differ between invocations" + tail;
			String note = fatal ?
				"invocations that skip it leave the others in the group waiting forever" :
				"the result is undefined, use 'sample_lod'";
			const auto& code = codes::asl("divergent-control");
			_diags.emplace_back(
				(fatal ? ir::Diagnostic::error(code, std::move(message)) : ir::Diagnostic::warning(code, std::move(message))).at(
					spanIn(_mod, e.loc)
				).note(std::move(note))
			);
		}

		void walkExpr(ir::ExprRef ref_) {
			if (ref_.index >= _mod.exprs.size()) {
				return;
			}

			const auto& e = _mod.exprs[ref_.index];
			for (const auto operand : e.operands) {
				walkExpr(operand);
			}

			if (e.kind == ir::ExprKind::eBuiltin) {
				const auto* fn = ir::findBuiltinFunction(e.name);
				/* the divergent call site itself is not part of the chain */
				if (fn != nullptr && fn->needsUniformControl && (_recurse == not _chain.empty())) {
					warn(ref_);
				}

			} else if (e.kind == ir::ExprKind::eCall && _recurse && e.function.index < _mod.functions.size()) {
				if (_uniformity.requiresUniform[e.function.index] && std::ranges::find(_chain, e.function.index) == _chain.end()) {
					_chain.emplace_back(e.function.index);
					visit(e.function.index, true, true);
					_chain.pop_back();
				}
			}
		}

		void walkStmt(ir::StmtRef ref_, bool whole_) {
			if (ref_.index >= _mod.stmts.size()) {
				return;
			}

			const auto& s = _mod.stmts[ref_.index];
			if (whole_ || not _uniformity.uniformStmt[ref_.index]) {
				walkExpr(s.lhs);
				walkExpr(s.rhs);
			}

			for (const auto child : s.children) {
				walkStmt(child, whole_);
			}
		}

		/* `whole_`: reached through a call made under lane-dependent control */
		void visit(size_t f_, bool recurse_, bool whole_) {
			const auto& fn = _mod.functions[f_];
			if (not fn.body.has_value()) {
				return;
			}

			_recurse = recurse_;
			walkStmt(fn.body->root, whole_);
		}
	};
}

ir::Diagnostics ir::typecheck(ref<Module> mod_) {
	auto diags = Checker { mod_ }.run();
	if (diags.empty()) {
		diags = DivergenceWarner { mod_ }.run();
	}
	return diags;
}
