#include <algorithm>
#include <bit>

#include "SpirvEmitter.Impl.hpp"
#include "../Builtin.hpp"

namespace hg::engine::accel::lang::ir::spirv {
	namespace {
		constexpr u32 none = ~0uL;

		[[nodiscard]] const TypeDesc& desc(const Module& mod_, TypeRef type_) {
			return mod_.types[type_.index];
		}

		[[nodiscard]] u32 width(const TypeDesc& desc_) noexcept {
			return desc_.kind == TypeKind::eVector ? desc_.n : 1uL;
		}

		[[nodiscard]] u32 component(char c_) noexcept {
			switch (c_) {
				case 'x':
				case 'r':
				case 's': return 0uL;
				case 'y':
				case 'g':
				case 't': return 1uL;
				case 'z':
				case 'b':
				case 'p': return 2uL;
				case 'w':
				case 'a':
				case 'q': return 3uL;
				default: return none;
			}
		}

		[[nodiscard]] u32 fieldIndex(const TypeDesc& desc_, StringView name_) {
			for (u32 i = 0uL; i < desc_.fields.size(); ++i) {
				if (desc_.fields[i].name == name_) {
					return i;
				}
			}
			return none;
		}

		/* a call or builtin anywhere below: the expression must not be evaluated speculatively */
		[[nodiscard]] bool impure(const Module& mod_, ExprRef expr_) {
			if (expr_.index >= mod_.exprs.size()) {
				return false;
			}
			const auto& e = mod_.exprs[expr_.index];
			if (e.kind == ExprKind::eCall || e.kind == ExprKind::eBuiltin) {
				return true;
			}
			return std::ranges::any_of(e.operands, [&](ExprRef o_) { return impure(mod_, o_); });
		}

		void internal(Impl& impl_, SourceLoc loc_, String message_) {
			impl_.fail(codes::asl("internal-spirv-emit"), loc_, std::move(message_));
		}
	}

	/* Emission primitives */

	u32 Impl::op(spv::Op op_, u32 resultType_, Ops operands_) {
		if (ended) {
			return 0uL;
		}
		const auto id = w.id();
		Ops all { resultType_, id };
		all.insert(all.end(), operands_.begin(), operands_.end());
		body.emplace_back(Instr { op_, std::move(all) });
		return id;
	}

	void Impl::code(spv::Op op_, Ops operands_) {
		if (not ended) {
			body.emplace_back(Instr { op_, std::move(operands_) });
		}
	}

	void Impl::terminate(spv::Op op_, Ops operands_) {
		if (not ended) {
			body.emplace_back(Instr { op_, std::move(operands_) });
			ended = true;
		}
	}

	void Impl::label(u32 label_) {
		// fall through into the new block
		terminate(spv::Op::OpBranch, Ops { label_ });
		body.emplace_back(Instr { spv::Op::OpLabel, Ops { label_ } });
		current = label_;
		ended = false;
	}

	u32 Impl::local(TypeRef type_) {
		const auto id = w.id();
		locals.emplace_back(Instr { spv::Op::OpVariable, Ops { pointer(spv::StorageClass::Function, type(type_)), id, w32(spv::StorageClass::Function) } });
		return id;
	}

	/* Functions */

	void Impl::bodies() {
		for (u32 i = 0uL; i < functions.size(); ++i) {
			const auto& fn = functions[i];
			if (fn.id == 0uL) {
				continue;
			}
			const auto& decl = mod.functions[i];

			locals.clear();
			body.clear();
			loops.clear();
			scope.clear();
			ended = false;
			const auto first = w.id();
			current = first;

			for (u32 p = 0uL; p < decl.params.size(); ++p) {
				const auto& param = decl.params[p];
				auto base = fn.params[p];
				if (param.kind == ParamKind::eValue) {
					base = local(param.type);
					code(spv::Op::OpStore, Ops { base, fn.params[p] });
				}
				scope.emplace_back(param.name, Place { .base = base, .basePointee = type(param.type), .type = param.type });
			}

			if (decl.body.has_value()) {
				statement(decl.body->root);
			} else {
				internal(*this, decl.loc, "reached function '" + decl.name + "' has no body");
			}
			terminate(fn.ret == voidType() ? spv::Op::OpReturn : spv::Op::OpUnreachable, Ops {});

			w.function(spv::Op::OpFunction, Ops { fn.ret, fn.id, 0uL, fn.type });
			for (u32 p = 0uL; p < fn.params.size(); ++p) {
				w.function(spv::Op::OpFunctionParameter, Ops { fn.paramTypes[p], fn.params[p] });
			}
			w.function(spv::Op::OpLabel, Ops { first });
			for (const auto& instr : locals) {
				w.function(instr.op, instr.operands);
			}
			for (const auto& instr : body) {
				w.function(instr.op, instr.operands);
			}
			w.function(spv::Op::OpFunctionEnd, Ops {});
		}
	}

	/* Statements */

	void Impl::statement(StmtRef stmt_) {
		if (ended || stmt_.index >= mod.stmts.size()) {
			return;
		}
		const auto& s = mod.stmts[stmt_.index];
		if (tier >= Tier::eDev && s.kind != StmtKind::eBlock && s.loc.source < debugFiles.size()) {
			code(spv::Op::OpLine, Ops { debugFiles[s.loc.source], s.loc.line, s.loc.column });
		}
		const auto child = [&](u32 i_) { return i_ < s.children.size() ? s.children[i_] : StmtRef {}; };
		const auto scoped = [&](StmtRef ref_) {
			const auto mark = scope.size();
			statement(ref_);
			scope.erase(scope.begin() + static_cast<std::ptrdiff_t>(mark), scope.end());
		};

		switch (s.kind) {
			case StmtKind::eDeclareLocal: {
				const auto var = local(s.type);
				nameOf(var, s.name);
				const Place place_ { .base = var, .basePointee = type(s.type), .type = s.type };
				if (s.lhs.index != none) {
					store(place_, value(s.lhs));
				}
				scope.emplace_back(s.name, place_);
				break;
			}
			case StmtKind::eAssign: {
				const auto& lhs = mod.exprs[s.lhs.index];
				const auto& rhs = mod.exprs[s.rhs.index];
				if (lhs.kind == ExprKind::eField && lhs.name.size() > 1uL &&
					desc(mod, mod.exprs[lhs.operands[0].index].type).kind == TypeKind::eVector) {
					// multi-component swizzle: load, shuffle the new components in, store back
					const auto vecType = mod.exprs[lhs.operands[0].index].type;
					const auto n = desc(mod, vecType).n;
					const auto target = place(lhs.operands[0]);
					if (target.base == 0uL) {
						internal(*this, s.loc, "assignment to a non-addressable swizzle");
						break;
					}
					// rhs first: a call in it may write the vector through `&mut`
					const auto rv = value(s.rhs);
					const auto vec = load(target);
					Ops comps {};
					for (const auto c : lhs.name) {
						comps.emplace_back(component(c));
					}
					u32 v = rv;
					if (s.op != Op::eNone) {
						Ops shuffle { vec, vec };
						shuffle.insert(shuffle.end(), comps.begin(), comps.end());
						const auto cur = op(spv::Op::OpVectorShuffle, type(lhs.type), shuffle);
						v = binary(s.op, lhs.type, lhs.type, cur, rhs.type, rv, s.loc);
					}
					Ops merged { vec, v };
					for (u32 i = 0uL; i < n; ++i) {
						const auto at = std::ranges::find(comps, i);
						merged.emplace_back(at != comps.end() ? n + static_cast<u32>(at - comps.begin()) : i);
					}
					store(target, op(spv::Op::OpVectorShuffle, type(vecType), merged));
					break;
				}
				const auto target = place(s.lhs);
				if (target.base == 0uL) {
					internal(*this, s.loc, "assignment to a non-addressable place");
					break;
				}
				u32 v;
				if (s.op == Op::eNone) {
					v = value(s.rhs);
				} else {
					const auto cur = load(target);
					v = binary(s.op, lhs.type, lhs.type, cur, rhs.type, value(s.rhs), s.loc);
				}
				store(target, v);
				break;
			}
			case StmtKind::eExpr: {
				(void)value(s.lhs);
				break;
			}
			case StmtKind::eReturn: {
				if (s.lhs.index == none) {
					terminate(spv::Op::OpReturn, Ops {});
				} else {
					const auto v = value(s.lhs);
					terminate(spv::Op::OpReturnValue, Ops { v });
				}
				break;
			}
			case StmtKind::eBlock: {
				const auto mark = scope.size();
				for (const auto c : s.children) {
					statement(c);
				}
				scope.erase(scope.begin() + static_cast<std::ptrdiff_t>(mark), scope.end());
				break;
			}
			case StmtKind::eIf: {
				const auto cond = value(s.lhs);
				const auto then = w.id();
				const auto merge = w.id();
				const auto hasElse = s.children.size() > 1uL;
				const auto other = hasElse ? w.id() : merge;
				code(spv::Op::OpSelectionMerge, Ops { merge, 0uL });
				terminate(spv::Op::OpBranchConditional, Ops { cond, then, other });
				label(then);
				scoped(child(0uL));
				terminate(spv::Op::OpBranch, Ops { merge });
				if (hasElse) {
					label(other);
					scoped(child(1uL));
					terminate(spv::Op::OpBranch, Ops { merge });
				}
				label(merge);
				break;
			}
			case StmtKind::eFor: {
				loop(child(0uL), s.lhs, child(1uL), child(2uL));
				break;
			}
			case StmtKind::eWhile: {
				loop({}, s.lhs, child(0uL), {});
				break;
			}
			case StmtKind::eBreak:
			case StmtKind::eContinue: {
				if (loops.empty()) {
					internal(*this, s.loc, "break/continue outside a loop");
					break;
				}
				terminate(spv::Op::OpBranch, Ops { s.kind == StmtKind::eBreak ? loops.back().merge : loops.back().cont });
				break;
			}
			case StmtKind::eDiscard: {
				terminate(spv::Op::OpTerminateInvocation, Ops {});
				break;
			}
			default: {
				internal(*this, s.loc, "statement kind not lowered before SPIR-V emission");
				break;
			}
		}
	}

	/*
	 * header: OpLoopMerge, branch to check; check: condition, break to merge; body: falls to cont;
	 * cont: the `for` step, back-edge to header. `continue` = branch to cont, `break` = branch to merge.
	 */
	void Impl::loop(StmtRef init_, ExprRef cond_, StmtRef body_, StmtRef step_) {
		const auto mark = scope.size();
		statement(init_);

		const auto header = w.id();
		const auto check = w.id();
		const auto entry = w.id();
		const auto cont = w.id();
		const auto merge = w.id();

		label(header);
		code(spv::Op::OpLoopMerge, Ops { merge, cont, 0uL });
		terminate(spv::Op::OpBranch, Ops { check });

		label(check);
		if (cond_.index != none) {
			const auto cond = value(cond_);
			terminate(spv::Op::OpBranchConditional, Ops { cond, entry, merge });
		}

		label(entry);
		loops.emplace_back(Loop { merge, cont });
		{
			const auto inner = scope.size();
			statement(body_);
			scope.erase(scope.begin() + static_cast<std::ptrdiff_t>(inner), scope.end());
		}
		loops.pop_back();

		label(cont);
		statement(step_);
		terminate(spv::Op::OpBranch, Ops { header });

		label(merge);
		scope.erase(scope.begin() + static_cast<std::ptrdiff_t>(mark), scope.end());
	}

	/* Places */

	Impl::Place Impl::place(ExprRef expr_) {
		// invariant: a non-addressable result (base 0) emitted nothing
		if (expr_.index >= mod.exprs.size()) {
			return {};
		}
		const auto& e = mod.exprs[expr_.index];
		switch (e.kind) {
			case ExprKind::eNameRef: {
				if (e.binding.index != none) {
					if (e.binding.index >= globals.size() || globals[e.binding.index].id == 0uL) {
						internal(*this, e.loc, "binding '" + e.name + "' has no variable");
						return {};
					}
					const auto& g = globals[e.binding.index];
					Place p {
						.base = g.id, .storage = g.storage, .explicitLayout = g.explicitLayout, .std140 = g.std140,
						.basePointee = g.pointee, .type = e.type
					};
					if (g.wrapped) {
						p.indices.emplace_back(u32Constant(0uL));
					}
					return p;
				}
				if (e.constant.index != none) {
					return {};
				}
				for (auto it = scope.rbegin(); it != scope.rend(); ++it) {
					if (it->first == e.name) {
						return it->second;
					}
				}
				return {};
			}
			case ExprKind::eField: {
				const auto& bd = desc(mod, mod.exprs[e.operands[0].index].type);
				auto index = none;
				if (bd.kind == TypeKind::eStruct) {
					index = fieldIndex(bd, e.name);
				} else if (bd.kind == TypeKind::eVector && e.name.size() == 1uL) {
					index = component(e.name[0]);
				}
				if (index == none) {
					return {};
				}
				auto p = place(e.operands[0]);
				if (p.base == 0uL) {
					return {};
				}
				p.indices.emplace_back(u32Constant(index));
				p.type = e.type;
				return p;
			}
			case ExprKind::eIndex: {
				auto p = place(e.operands[0]);
				if (p.base == 0uL) {
					return {};
				}
				p.indices.emplace_back(value(e.operands[1]));
				p.type = e.type;
				return p;
			}
			case ExprKind::eBuiltin: {
				return builtinPlace(expr_);
			}
			default: {
				return {};
			}
		}
	}

	u32 Impl::pointeeOf(ref<const Place> place_) {
		if (place_.indices.empty() && place_.basePointee != 0uL) {
			return place_.basePointee;
		}
		return place_.explicitLayout ? laidOut(place_.type, place_.std140) : type(place_.type);
	}

	u32 Impl::pointerOf(ref<const Place> place_) {
		if (place_.indices.empty()) {
			return place_.base;
		}
		Ops operands { place_.base };
		operands.insert(operands.end(), place_.indices.begin(), place_.indices.end());
		return op(spv::Op::OpAccessChain, pointer(place_.storage, pointeeOf(place_)), operands);
	}

	u32 Impl::load(ref<const Place> place_) {
		const auto ptr = pointerOf(place_);
		const auto pointee = pointeeOf(place_);
		const auto v = op(spv::Op::OpLoad, pointee, Ops { ptr });
		return fromLaid(place_.type, pointee, v, place_.std140);
	}

	void Impl::store(ref<const Place> place_, u32 value_) {
		const auto ptr = pointerOf(place_);
		const auto pointee = pointeeOf(place_);
		code(spv::Op::OpStore, Ops { ptr, toLaid(place_.type, pointee, value_, place_.std140) });
	}

	/* explicit layouts store bool as u32 (bvec as uvec); composites convert member-wise */
	u32 Impl::fromLaid(TypeRef type_, u32 laid_, u32 value_, bool std140_) {
		const auto id = type(type_);
		if (laid_ == id) {
			return value_;
		}
		const auto& d = desc(mod, type_);
		if ((d.kind == TypeKind::eScalar || d.kind == TypeKind::eVector) && d.scalar == ScalarKind::eBool) {
			return op(spv::Op::OpINotEqual, id, Ops { value_, w.constant(spv::Op::OpConstantNull, laid_, Ops {}) });
		}
		Ops parts {};
		if (d.kind == TypeKind::eStruct) {
			for (u32 i = 0uL; i < d.fields.size(); ++i) {
				const auto ft = d.fields[i].type;
				const auto fl = laidOut(ft, std140_);
				parts.emplace_back(fromLaid(ft, fl, op(spv::Op::OpCompositeExtract, fl, Ops { value_, i }), std140_));
			}
		} else if (d.kind == TypeKind::eArray && d.count > 0uL) {
			const auto el = laidOut(d.element, std140_);
			for (u32 i = 0uL; i < d.count; ++i) {
				parts.emplace_back(fromLaid(d.element, el, op(spv::Op::OpCompositeExtract, el, Ops { value_, i }), std140_));
			}
		} else {
			return op(spv::Op::OpCopyLogical, id, Ops { value_ });
		}
		return op(spv::Op::OpCompositeConstruct, id, parts);
	}

	u32 Impl::toLaid(TypeRef type_, u32 laid_, u32 value_, bool std140_) {
		const auto id = type(type_);
		if (laid_ == id) {
			return value_;
		}
		const auto& d = desc(mod, type_);
		if ((d.kind == TypeKind::eScalar || d.kind == TypeKind::eVector) && d.scalar == ScalarKind::eBool) {
			const auto n = width(d);
			return op(spv::Op::OpSelect, laid_, Ops { value_, number(ScalarKind::eUInt, 32uL, n, true), number(ScalarKind::eUInt, 32uL, n, false) });
		}
		Ops parts {};
		if (d.kind == TypeKind::eStruct) {
			for (u32 i = 0uL; i < d.fields.size(); ++i) {
				const auto ft = d.fields[i].type;
				parts.emplace_back(toLaid(ft, laidOut(ft, std140_), op(spv::Op::OpCompositeExtract, type(ft), Ops { value_, i }), std140_));
			}
		} else if (d.kind == TypeKind::eArray && d.count > 0uL) {
			const auto el = laidOut(d.element, std140_);
			for (u32 i = 0uL; i < d.count; ++i) {
				parts.emplace_back(toLaid(d.element, el, op(spv::Op::OpCompositeExtract, type(d.element), Ops { value_, i }), std140_));
			}
		} else {
			return op(spv::Op::OpCopyLogical, laid_, Ops { value_ });
		}
		return op(spv::Op::OpCompositeConstruct, laid_, parts);
	}

	/* Expressions */

	u32 Impl::value(ExprRef expr_) {
		if (expr_.index >= mod.exprs.size()) {
			internal(*this, {}, "missing expression");
			return 0uL;
		}
		const auto& e = mod.exprs[expr_.index];
		switch (e.kind) {
			case ExprKind::eLiteral: {
				return literal(e.type, e.literal);
			}
			case ExprKind::eNameRef: {
				if (e.constant.index != none) {
					if (e.constant.index < constants.size() && constants[e.constant.index] != 0uL) {
						return constants[e.constant.index];
					}
					internal(*this, e.loc, "constant '" + e.name + "' was not declared");
					return 0uL;
				}
				const auto p = place(expr_);
				if (p.base != 0uL) {
					return load(p);
				}
				if (e.binding.index == none) {
					internal(*this, e.loc, "unknown name '" + e.name + "'");
				}
				return 0uL;
			}
			case ExprKind::eField: {
				const auto p = place(expr_);
				return p.base != 0uL ? load(p) : field(expr_);
			}
			case ExprKind::eIndex: {
				const auto p = place(expr_);
				if (p.base != 0uL) {
					return load(p);
				}
				const auto baseType = mod.exprs[e.operands[0].index].type;
				const auto base = value(e.operands[0]);
				const auto index = value(e.operands[1]);
				if (desc(mod, baseType).kind == TypeKind::eVector) {
					return op(spv::Op::OpVectorExtractDynamic, type(e.type), Ops { base, index });
				}
				// dynamic index into a value: spill to a local
				const auto tmp = local(baseType);
				code(spv::Op::OpStore, Ops { tmp, base });
				const auto ptr = op(spv::Op::OpAccessChain, pointer(spv::StorageClass::Function, type(e.type)), Ops { tmp, index });
				return op(spv::Op::OpLoad, type(e.type), Ops { ptr });
			}
			case ExprKind::eCall: {
				return call(e);
			}
			case ExprKind::eUnary: {
				const auto v = value(e.operands[0]);
				const auto& d = desc(mod, e.type);
				const auto id = type(e.type);
				switch (e.op) {
					case Op::eNeg: {
						if (d.kind == TypeKind::eMatrix) {
							const auto column = shaped(ScalarKind::eFloat, d.bits, d.m);
							Ops columns {};
							for (u32 j = 0uL; j < d.n; ++j) {
								const auto c = op(spv::Op::OpCompositeExtract, column, Ops { v, j });
								columns.emplace_back(op(spv::Op::OpFNegate, column, Ops { c }));
							}
							return op(spv::Op::OpCompositeConstruct, id, columns);
						}
						return op(d.scalar == ScalarKind::eFloat ? spv::Op::OpFNegate : spv::Op::OpSNegate, id, Ops { v });
					}
					case Op::eNot: return op(spv::Op::OpLogicalNot, id, Ops { v });
					case Op::eBitNot: return op(spv::Op::OpNot, id, Ops { v });
					default: {
						internal(*this, e.loc, "unsupported unary operator");
						return 0uL;
					}
				}
			}
			case ExprKind::eBinary: {
				if (e.op == Op::eLogicalAnd || e.op == Op::eLogicalOr) {
					return shortCircuit(e);
				}
				const auto l = value(e.operands[0]);
				const auto r = value(e.operands[1]);
				return binary(e.op, e.type, mod.exprs[e.operands[0].index].type, l, mod.exprs[e.operands[1].index].type, r, e.loc);
			}
			case ExprKind::eConstruct: {
				return construct(e);
			}
			case ExprKind::eBuiltin: {
				return builtin(expr_);
			}
			case ExprKind::eTernary: {
				return ternary(e);
			}
			default: {
				internal(*this, e.loc, "expression kind not lowered before SPIR-V emission");
				return 0uL;
			}
		}
	}

	u32 Impl::field(ExprRef expr_) {
		const auto& e = mod.exprs[expr_.index];
		const auto& bd = desc(mod, mod.exprs[e.operands[0].index].type);
		const auto v = value(e.operands[0]);
		if (bd.kind == TypeKind::eStruct) {
			const auto index = fieldIndex(bd, e.name);
			if (index != none) {
				return op(spv::Op::OpCompositeExtract, type(e.type), Ops { v, index });
			}
		} else if (bd.kind == TypeKind::eVector) {
			Ops comps {};
			for (const auto c : e.name) {
				comps.emplace_back(component(c));
			}
			if (std::ranges::find(comps, none) == comps.end()) {
				if (comps.size() == 1uL) {
					return op(spv::Op::OpCompositeExtract, type(e.type), Ops { v, comps[0] });
				}
				Ops shuffle { v, v };
				shuffle.insert(shuffle.end(), comps.begin(), comps.end());
				return op(spv::Op::OpVectorShuffle, type(e.type), shuffle);
			}
		}
		internal(*this, e.loc, "unknown member '" + e.name + "'");
		return 0uL;
	}

	u32 Impl::shortCircuit(ref<const Expr> expr_) {
		const auto isAnd = expr_.op == Op::eLogicalAnd;
		const auto l = value(expr_.operands[0]);
		const auto lEnd = current;
		const auto rhs = w.id();
		const auto merge = w.id();
		code(spv::Op::OpSelectionMerge, Ops { merge, 0uL });
		terminate(spv::Op::OpBranchConditional, isAnd ? Ops { l, rhs, merge } : Ops { l, merge, rhs });
		label(rhs);
		const auto r = value(expr_.operands[1]);
		const auto rEnd = current;
		terminate(spv::Op::OpBranch, Ops { merge });
		label(merge);
		const auto skipped = w.constant(isAnd ? spv::Op::OpConstantFalse : spv::Op::OpConstantTrue, boolType(), Ops {});
		return op(spv::Op::OpPhi, boolType(), Ops { skipped, lEnd, r, rEnd });
	}

	u32 Impl::ternary(ref<const Expr> expr_) {
		const auto cond = value(expr_.operands[0]);
		const auto id = type(expr_.type);
		if (not impure(mod, expr_.operands[1]) && not impure(mod, expr_.operands[2])) {
			const auto a = value(expr_.operands[1]);
			const auto b = value(expr_.operands[2]);
			return op(spv::Op::OpSelect, id, Ops { cond, a, b });
		}
		const auto then = w.id();
		const auto other = w.id();
		const auto merge = w.id();
		code(spv::Op::OpSelectionMerge, Ops { merge, 0uL });
		terminate(spv::Op::OpBranchConditional, Ops { cond, then, other });
		label(then);
		const auto a = value(expr_.operands[1]);
		const auto aEnd = current;
		terminate(spv::Op::OpBranch, Ops { merge });
		label(other);
		const auto b = value(expr_.operands[2]);
		const auto bEnd = current;
		terminate(spv::Op::OpBranch, Ops { merge });
		label(merge);
		return op(spv::Op::OpPhi, id, Ops { a, aEnd, b, bEnd });
	}

	u32 Impl::binary(Op op_, TypeRef result_, TypeRef lt_, u32 l_, TypeRef rt_, u32 r_, SourceLoc loc_) {
		const auto& ld = desc(mod, lt_);
		const auto& rd = desc(mod, rt_);
		const auto id = type(result_);

		if (ld.kind == TypeKind::eMatrix || rd.kind == TypeKind::eMatrix) {
			const auto lm = ld.kind == TypeKind::eMatrix;
			const auto rm = rd.kind == TypeKind::eMatrix;
			if (op_ == Op::eMul) {
				if (lm && rm) {
					return op(spv::Op::OpMatrixTimesMatrix, id, Ops { l_, r_ });
				}
				if (lm && rd.kind == TypeKind::eVector) {
					return op(spv::Op::OpMatrixTimesVector, id, Ops { l_, r_ });
				}
				if (rm && ld.kind == TypeKind::eVector) {
					return op(spv::Op::OpVectorTimesMatrix, id, Ops { l_, r_ });
				}
				return lm ? op(spv::Op::OpMatrixTimesScalar, id, Ops { l_, r_ }) : op(spv::Op::OpMatrixTimesScalar, id, Ops { r_, l_ });
			}
			if (op_ == Op::eAdd || op_ == Op::eSub || op_ == Op::eDiv) {
				// column-wise; typecheck admits mat op mat and mat op scalar (either side), the scalar is splat per column
				const auto& md = lm ? ld : rd;
				const auto code_ = op_ == Op::eAdd ? spv::Op::OpFAdd : op_ == Op::eSub ? spv::Op::OpFSub : spv::Op::OpFDiv;
				const auto column = shaped(ScalarKind::eFloat, md.bits, md.m);
				const auto operand = [&](u32 v_, bool matrix_, u32 j_) {
					return matrix_ ? op(spv::Op::OpCompositeExtract, column, Ops { v_, j_ }) : op(spv::Op::OpCompositeConstruct, column, Ops(md.m, v_));
				};
				Ops columns {};
				for (u32 j = 0uL; j < md.n; ++j) {
					columns.emplace_back(op(code_, column, Ops { operand(l_, lm, j), operand(r_, rm, j) }));
				}
				return op(spv::Op::OpCompositeConstruct, id, columns);
			}
			internal(*this, loc_, "unsupported matrix operator");
			return 0uL;
		}

		const auto kind = ld.scalar;
		const auto n = std::max(width(ld), width(rd));
		if (op_ == Op::eMul && kind == ScalarKind::eFloat && width(ld) != width(rd)) {
			return width(ld) > 1uL ? op(spv::Op::OpVectorTimesScalar, id, Ops { l_, r_ }) : op(spv::Op::OpVectorTimesScalar, id, Ops { r_, l_ });
		}
		const auto splat = [&](u32 v_, const TypeDesc& d_) {
			return width(d_) == n ? v_ : op(spv::Op::OpCompositeConstruct, shaped(d_.scalar, d_.bits, n), Ops(n, v_));
		};
		const auto l = splat(l_, ld);
		const auto r = splat(r_, rd);
		const auto f = kind == ScalarKind::eFloat;
		const auto s = kind == ScalarKind::eInt;
		const auto b = kind == ScalarKind::eBool;

		const auto compare = [&](spv::Op code_) {
			auto v = op(code_, shaped(ScalarKind::eBool, 32uL, n), Ops { l, r });
			if (n > 1uL && desc(mod, result_).kind == TypeKind::eScalar) {
				v = op(op_ == Op::eNe ? spv::Op::OpAny : spv::Op::OpAll, boolType(), Ops { v });
			}
			return v;
		};

		switch (op_) {
			case Op::eAdd: return op(f ? spv::Op::OpFAdd : spv::Op::OpIAdd, id, Ops { l, r });
			case Op::eSub: return op(f ? spv::Op::OpFSub : spv::Op::OpISub, id, Ops { l, r });
			case Op::eMul: return op(f ? spv::Op::OpFMul : spv::Op::OpIMul, id, Ops { l, r });
			case Op::eDiv: return op(f ? spv::Op::OpFDiv : s ? spv::Op::OpSDiv : spv::Op::OpUDiv, id, Ops { l, r });
			// signed % is OpSRem: result takes the sign of the dividend, as with C/GLSL truncating division
			case Op::eMod: return op(f ? spv::Op::OpFRem : s ? spv::Op::OpSRem : spv::Op::OpUMod, id, Ops { l, r });
			case Op::eBitAnd: return op(spv::Op::OpBitwiseAnd, id, Ops { l, r });
			case Op::eBitOr: return op(spv::Op::OpBitwiseOr, id, Ops { l, r });
			case Op::eBitXor: return op(spv::Op::OpBitwiseXor, id, Ops { l, r });
			case Op::eShl: return op(spv::Op::OpShiftLeftLogical, id, Ops { l, r });
			case Op::eShr: return op(s ? spv::Op::OpShiftRightArithmetic : spv::Op::OpShiftRightLogical, id, Ops { l, r });
			case Op::eLogicalAnd: return op(spv::Op::OpLogicalAnd, id, Ops { l, r });
			case Op::eLogicalOr: return op(spv::Op::OpLogicalOr, id, Ops { l, r });
			case Op::eEq: return compare(b ? spv::Op::OpLogicalEqual : f ? spv::Op::OpFOrdEqual : spv::Op::OpIEqual);
			case Op::eNe: return compare(b ? spv::Op::OpLogicalNotEqual : f ? spv::Op::OpFUnordNotEqual : spv::Op::OpINotEqual);
			case Op::eLt: return compare(f ? spv::Op::OpFOrdLessThan : s ? spv::Op::OpSLessThan : spv::Op::OpULessThan);
			case Op::eLe: return compare(f ? spv::Op::OpFOrdLessThanEqual : s ? spv::Op::OpSLessThanEqual : spv::Op::OpULessThanEqual);
			case Op::eGt: return compare(f ? spv::Op::OpFOrdGreaterThan : s ? spv::Op::OpSGreaterThan : spv::Op::OpUGreaterThan);
			case Op::eGe: return compare(f ? spv::Op::OpFOrdGreaterThanEqual : s ? spv::Op::OpSGreaterThanEqual : spv::Op::OpUGreaterThanEqual);
			default: {
				internal(*this, loc_, "unsupported binary operator");
				return 0uL;
			}
		}
	}

	u32 Impl::construct(ref<const Expr> expr_) {
		const auto& t = desc(mod, expr_.type);
		const auto id = type(expr_.type);
		Ops args {};
		for (const auto o : expr_.operands) {
			args.emplace_back(value(o));
		}
		if (args.empty()) {
			return w.constant(spv::Op::OpConstantNull, id, Ops {});
		}
		const auto firstType = mod.exprs[expr_.operands[0].index].type;
		const auto& first = desc(mod, firstType);
		const auto numeric = [](const TypeDesc& d_) { return d_.kind == TypeKind::eScalar || d_.kind == TypeKind::eVector; };

		if (args.size() == 1uL) {
			if (firstType == expr_.type) {
				return args[0];
			}
			if (numeric(t) && numeric(first) && width(first) == width(t)) {
				return convert(firstType, args[0], t.scalar, t.bits, expr_.loc);
			}
			if (t.kind == TypeKind::eVector && first.kind == TypeKind::eScalar) {
				const auto c = convert(firstType, args[0], t.scalar, t.bits, expr_.loc);
				return op(spv::Op::OpCompositeConstruct, id, Ops(t.n, c));
			}
			if (t.kind == TypeKind::eMatrix && first.kind == TypeKind::eScalar) {
				// diagonal matrix
				const auto s = convert(firstType, args[0], ScalarKind::eFloat, t.bits, expr_.loc);
				const auto zero = number(ScalarKind::eFloat, t.bits, 1uL, false);
				const auto column = shaped(ScalarKind::eFloat, t.bits, t.m);
				Ops columns {};
				for (u32 j = 0uL; j < t.n; ++j) {
					Ops parts(t.m, zero);
					if (j < t.m) {
						parts[j] = s;
					}
					columns.emplace_back(op(spv::Op::OpCompositeConstruct, column, parts));
				}
				return op(spv::Op::OpCompositeConstruct, id, columns);
			}
		}

		if (t.kind == TypeKind::eMatrix && first.kind == TypeKind::eScalar && args.size() == t.n * t.m) {
			const auto column = shaped(ScalarKind::eFloat, t.bits, t.m);
			Ops columns {};
			for (u32 j = 0uL; j < t.n; ++j) {
				Ops parts {};
				for (u32 k = 0uL; k < t.m; ++k) {
					parts.emplace_back(convert(mod.exprs[expr_.operands[j * t.m + k].index].type, args[j * t.m + k], ScalarKind::eFloat, t.bits, expr_.loc));
				}
				columns.emplace_back(op(spv::Op::OpCompositeConstruct, column, parts));
			}
			return op(spv::Op::OpCompositeConstruct, id, columns);
		}

		if (t.kind == TypeKind::eVector) {
			for (u32 i = 0uL; i < args.size(); ++i) {
				const auto ot = mod.exprs[expr_.operands[i].index].type;
				if (numeric(desc(mod, ot))) {
					args[i] = convert(ot, args[i], t.scalar, t.bits, expr_.loc);
				}
			}
		}
		return op(spv::Op::OpCompositeConstruct, id, args);
	}

	u32 Impl::convert(TypeRef from_, u32 value_, ScalarKind kind_, u32 bits_, SourceLoc loc_) {
		(void)loc_;
		const auto& fd = desc(mod, from_);
		const auto n = width(fd);
		const auto fk = fd.scalar;
		const auto fb = fd.bits;
		const auto to = shaped(kind_, bits_, n);

		if (fk == kind_ && (kind_ == ScalarKind::eBool || fb == bits_)) {
			return value_;
		}
		if (kind_ == ScalarKind::eBool) {
			return op(fk == ScalarKind::eFloat ? spv::Op::OpFUnordNotEqual : spv::Op::OpINotEqual, to, Ops { value_, number(fk, fb, n, false) });
		}
		if (fk == ScalarKind::eBool) {
			return op(spv::Op::OpSelect, to, Ops { value_, number(kind_, bits_, n, true), number(kind_, bits_, n, false) });
		}
		if (fk == ScalarKind::eFloat) {
			return op(
				kind_ == ScalarKind::eFloat ? spv::Op::OpFConvert : kind_ == ScalarKind::eInt ? spv::Op::OpConvertFToS : spv::Op::OpConvertFToU,
				to, Ops { value_ }
			);
		}
		if (kind_ == ScalarKind::eFloat) {
			return op(fk == ScalarKind::eInt ? spv::Op::OpConvertSToF : spv::Op::OpConvertUToF, to, Ops { value_ });
		}
		if (fb == bits_) {
			return op(spv::Op::OpBitcast, to, Ops { value_ });
		}
		if (fk == ScalarKind::eInt) {
			return op(spv::Op::OpSConvert, to, Ops { value_ });
		}
		// OpUConvert must produce an unsigned type
		const auto wide = op(spv::Op::OpUConvert, shaped(ScalarKind::eUInt, bits_, n), Ops { value_ });
		return kind_ == ScalarKind::eUInt ? wide : op(spv::Op::OpBitcast, to, Ops { wide });
	}

	u32 Impl::call(ref<const Expr> expr_) {
		const auto index = expr_.function.index;
		if (index >= functions.size() || functions[index].id == 0uL) {
			internal(*this, expr_.loc, "call of an unreached function '" + expr_.name + "'");
			return 0uL;
		}
		const auto& decl = mod.functions[index];
		Ops args { functions[index].id };
		Vector<std::pair<Place, u32>> copyBack {};

		for (u32 i = 0uL; i < expr_.operands.size() && i < decl.params.size(); ++i) {
			const auto& param = decl.params[i];
			const auto arg = expr_.operands[i];
			if (param.kind == ParamKind::eValue) {
				args.emplace_back(value(arg));
				continue;
			}
			auto p = place(arg);
			// logical addressing: only a whole memory object may be passed as a pointer argument
			if (p.base != 0uL && p.storage == spv::StorageClass::Function && not p.explicitLayout && p.indices.empty()) {
				args.emplace_back(p.base);
				continue;
			}
			const auto tmp = local(param.type);
			const auto v = p.base != 0uL ? load(p) : value(arg);
			code(spv::Op::OpStore, Ops { tmp, v });
			if (p.base != 0uL && param.kind == ParamKind::eRefMut) {
				copyBack.emplace_back(std::move(p), tmp);
			}
			args.emplace_back(tmp);
		}

		const auto result = op(spv::Op::OpFunctionCall, functions[index].ret, args);
		for (const auto& [p, tmp] : copyBack) {
			store(p, op(spv::Op::OpLoad, type(p.type), Ops { tmp }));
		}
		return result;
	}

	/* Builtin library (SPIR-V forms in Builtin.hpp) */

	u32 Impl::glslStd() {
		if (glsl450 == 0uL) {
			glsl450 = w.id();
			Ops ops { glsl450 };
			SpirvWriter::string(ops, "GLSL.std.450");
			w.extInstImport(spv::Op::OpExtInstImport, ops);
		}
		return glsl450;
	}

	u32 Impl::builtin(ExprRef expr_) {
		const auto& e = mod.exprs[expr_.index];
		if (e.name == "bitcast" && e.operands.size() == 1uL) {
			const auto v = value(e.operands[0]);
			return op(spv::Op::OpBitcast, type(e.type), Ops { v });
		}
		if (findBuiltinVariable(e.name) != nullptr) {
			const auto p = builtinPlace(expr_);
			return p.base != 0uL ? load(p) : 0uL;
		}

		const auto& sigs = parsedBuiltinSignatures();
		const auto operandDesc = [&](u32 i_) -> const TypeDesc& { return desc(mod, mod.exprs[e.operands[i_].index].type); };
		const auto familyKind = [](const BuiltinSigType& t_) {
			switch (t_.family) {
				case BuiltinFamily::eGenF:
				case BuiltinFamily::eGenH:
				case BuiltinFamily::eMat: return ScalarKind::eFloat;
				case BuiltinFamily::eGenI: return ScalarKind::eInt;
				case BuiltinFamily::eGenU: return ScalarKind::eUInt;
				case BuiltinFamily::eGenB: return ScalarKind::eBool;
				default: return t_.scalar;
			}
		};

		// overloads share a name; the typechecked first operand picks the row (`abs` FAbs vs SAbs, atomics U vs S)
		auto row = ~0uL;
		for (u32 i = 0uL; i < std::size(builtinFunctions); ++i) {
			if (builtinFunctions[i].name != e.name || static_cast<size_t>(sigs[i].count) != e.operands.size()) {
				continue;
			}
			if (row == ~0uL) {
				row = i;
			}
			if (e.operands.empty() || familyKind(sigs[i].params[0]) == operandDesc(0uL).scalar) {
				row = i;
				break;
			}
		}
		if (row == ~0uL) {
			internal(*this, e.loc, "builtin '" + e.name + "' has no matching overload");
			return 0uL;
		}

		const auto& form = builtinFunctionSpirv[row].form;
		require(form.capability);
		if (not form.extension.empty()) {
			extensions.emplace(form.extension);
		}

		// GLSL.std.450 / OpFMod operands match the result type: scalar arguments of vector calls are splatted
		const auto args = [&](bool splat_) {
			Ops out {};
			for (u32 i = 0uL; i < e.operands.size(); ++i) {
				auto v = value(e.operands[i]);
				if (not splat_) {
					out.emplace_back(v);
					continue;
				}
				// void calls have no result type to read
				const auto& rd = desc(mod, e.type);
				const auto& ad = operandDesc(i);
				if (rd.kind == TypeKind::eVector && ad.kind == TypeKind::eScalar) {
					v = op(spv::Op::OpCompositeConstruct, shaped(ad.scalar, ad.bits, rd.n), Ops(rd.n, v));
				}
				out.emplace_back(v);
			}
			return out;
		};
		const auto workgroup = [&] { return u32Constant(w32(spv::Scope::Workgroup)); };
		const auto sharedRelease = [&] {
			return u32Constant(w32(spv::MemorySemanticsMask::AcquireRelease) | w32(spv::MemorySemanticsMask::WorkgroupMemory));
		};

		switch (form.kind) {
			case SpirvFormKind::eExtInst: {
				auto ops = Ops { glslStd(), form.code };
				const auto a = args(true);
				ops.insert(ops.end(), a.begin(), a.end());
				return op(spv::Op::OpExtInst, type(e.type), std::move(ops));
			}
			case SpirvFormKind::eCore: {
				const auto code_ = static_cast<spv::Op>(form.code);
				if (e.name.starts_with("atomic_")) {
					// GLSL semantics: device scope, relaxed
					const auto ptr = pointerOf(place(e.operands[0]));
					const auto scope = u32Constant(w32(spv::Scope::Device));
					const auto relaxed = u32Constant(w32(spv::MemorySemanticsMask::MaskNone));
					if (code_ == spv::Op::OpAtomicCompareExchange) {
						const auto compare = value(e.operands[1]);
						const auto desired = value(e.operands[2]);
						return op(code_, type(e.type), Ops { ptr, scope, relaxed, relaxed, desired, compare });
					}
					const auto v = value(e.operands[1]);
					return op(code_, type(e.type), Ops { ptr, scope, relaxed, v });
				}
				if (sigs[row].ret.family == BuiltinFamily::eVoid) {
					code(code_, args(false));
					return 0uL;
				}
				return op(code_, type(e.type), args(code_ == spv::Op::OpFMod));
			}
			case SpirvFormKind::eLower: {
				break;
			}
		}

		switch (static_cast<BuiltinLowering>(form.code)) {
			case BuiltinLowering::eSample:
			case BuiltinLowering::eSampleLod:
			case BuiltinLowering::eSampleSparse:
			case BuiltinLowering::eSampleSparseClamp: {
				const auto lowering = static_cast<BuiltinLowering>(form.code);
				const auto texture = e.operands[0];
				// every operand before OpSampledImage: its result must be consumed in the same block
				const auto img = value(texture);
				const auto smp = value(e.operands[1]);
				const auto coord = value(e.operands[2]);
				const auto extra = lowering == BuiltinLowering::eSampleLod || lowering == BuiltinLowering::eSampleSparseClamp ? value(e.operands[3]) : 0uL;
				const auto sampledType = w.type(spv::Op::OpTypeSampledImage, Ops { type(mod.exprs[texture.index].type) });
				const auto sampled = op(spv::Op::OpSampledImage, sampledType, Ops { img, smp });
				const auto vec4 = shaped(ScalarKind::eFloat, 32uL, 4uL);
				// implicit lod needs derivatives (fragment only); elsewhere GLSL texture() samples lod 0
				const auto implicit = mod.entry == EntryKind::eFragment;
				const auto zero = number(ScalarKind::eFloat, 32uL, 1uL, false);
				switch (lowering) {
					case BuiltinLowering::eSample: {
						if (not implicit) {
							return op(spv::Op::OpImageSampleExplicitLod, vec4, Ops { sampled, coord, w32(spv::ImageOperandsMask::Lod), zero });
						}
						return op(spv::Op::OpImageSampleImplicitLod, vec4, Ops { sampled, coord });
					}
					case BuiltinLowering::eSampleLod: {
						return op(spv::Op::OpImageSampleExplicitLod, vec4, Ops { sampled, coord, w32(spv::ImageOperandsMask::Lod), extra });
					}
					default: {
						require(spv::Capability::SparseResidency);
						auto ops = Ops { sampled, coord };
						const auto clamp = lowering == BuiltinLowering::eSampleSparseClamp;
						if (implicit && clamp) {
							ops.insert(ops.end(), { w32(spv::ImageOperandsMask::MinLod), extra });
						} else if (not implicit) {
							// MinLod is invalid with an explicit Lod: lod 0 clamped from below is max(0, clamp)
							const auto f32 = scalar(ScalarKind::eFloat, 32uL);
							const auto lod = clamp ? op(spv::Op::OpExtInst, f32, Ops { glslStd(), w32(GLSLstd450FMax), zero, extra }) : zero;
							ops.insert(ops.end(), { w32(spv::ImageOperandsMask::Lod), lod });
						}
						const auto s32 = scalar(ScalarKind::eInt, 32uL);
						const auto result = op(implicit ? spv::Op::OpImageSparseSampleImplicitLod : spv::Op::OpImageSparseSampleExplicitLod, w.type(spv::Op::OpTypeStruct, Ops { s32, vec4 }), std::move(ops));
						store(place(e.operands.back()), op(spv::Op::OpCompositeExtract, vec4, Ops { result, 1uL }));
						return op(spv::Op::OpCompositeExtract, s32, Ops { result, 0uL });
					}
				}
			}
			case BuiltinLowering::eTextureSize: {
				const auto img = value(e.operands[0]);
				const auto lod = value(e.operands[2]);
				return op(spv::Op::OpImageQuerySizeLod, type(e.type), Ops { img, lod });
			}
			case BuiltinLowering::eDot: {
				const auto l = value(e.operands[0]);
				const auto r = value(e.operands[1]);
				return op(operandDesc(0uL).kind == TypeKind::eScalar ? spv::Op::OpFMul : spv::Op::OpDot, type(e.type), Ops { l, r });
			}
			case BuiltinLowering::eBarrier: {
				if (mod.entry == EntryKind::eTessCtrl) {
					// tess_ctrl orders the output patch writes only (execution barrier, no memory semantics)
					code(spv::Op::OpControlBarrier, Ops { workgroup(), u32Constant(w32(spv::Scope::Invocation)), u32Constant(w32(spv::MemorySemanticsMask::MaskNone)) });
				} else {
					code(spv::Op::OpControlBarrier, Ops { workgroup(), workgroup(), sharedRelease() });
				}
				return 0uL;
			}
			case BuiltinLowering::eMemoryBarrierShared: {
				code(spv::Op::OpMemoryBarrier, Ops { workgroup(), sharedRelease() });
				return 0uL;
			}
		}
		internal(*this, e.loc, "builtin '" + e.name + "' has no SPIR-V lowering");
		return 0uL;
	}

	Impl::Place Impl::builtinPlace(ExprRef expr_) {
		const auto& e = mod.exprs[expr_.index];
		const auto* var = findBuiltinVariable(e.name);
		if (var == nullptr) {
			internal(*this, e.loc, "builtin '" + e.name + "' is not a variable");
			return {};
		}
		if (const auto it = builtinVars.find(String { e.name }); it != builtinVars.end()) {
			return it->second;
		}

		const auto& spirv = builtinVariableSpirv[static_cast<size_t>(var - builtinVariables.data())];
		const auto storage = var->writable && (var->writeStages & stageBit(mod.entry)) != 0u ? spv::StorageClass::Output : spv::StorageClass::Input;
		auto pointee = shaped(var->scalar, 32uL, var->n);
		if (var->count > 0uL) {
			pointee = w.type(spv::Op::OpTypeArray, Ops { pointee, u32Constant(var->count) });
		} else if (var->arrayed) {
			pointee = w.type(spv::Op::OpTypeArray, Ops { pointee, u32Constant(vertexCount(storage == spv::StorageClass::Input)) });
		}

		const auto id = w.id();
		w.global(spv::Op::OpVariable, Ops { pointer(storage, pointee), id, w32(storage) });
		w.annotation(spv::Op::OpDecorate, Ops { id, w32(spv::Decoration::BuiltIn), w32(spirv.builtIn) });
		if (var->count > 0uL) {
			w.annotation(spv::Op::OpDecorate, Ops { id, w32(spv::Decoration::Patch) });
		}
		if (storage == spv::StorageClass::Input && mod.entry == EntryKind::eFragment && (var->scalar == ScalarKind::eInt || var->scalar == ScalarKind::eUInt)) {
			w.annotation(spv::Op::OpDecorate, Ops { id, w32(spv::Decoration::Flat) });
		}
		nameOf(id, var->name);
		interface.emplace_back(id);
		require(spirv.capability);
		if (not spirv.extension.empty()) {
			extensions.emplace(spirv.extension);
		}

		const Place p { .base = id, .storage = storage, .basePointee = pointee, .type = e.type };
		builtinVars.emplace(String { e.name }, p);
		return p;
	}

	/* Constants and types */

	u32 Impl::shaped(ScalarKind kind_, u32 bits_, u32 n_) {
		const auto s = scalar(kind_, bits_);
		return n_ <= 1uL ? s : w.type(spv::Op::OpTypeVector, Ops { s, n_ });
	}

	u32 Impl::number(ScalarKind kind_, u32 bits_, u32 n_, bool one_) {
		u32 c;
		if (kind_ == ScalarKind::eBool) {
			c = w.constant(one_ ? spv::Op::OpConstantTrue : spv::Op::OpConstantFalse, boolType(), Ops {});
		} else {
			Ops words {};
			if (kind_ == ScalarKind::eFloat && bits_ == 64uL) {
				const auto b = one_ ? std::bit_cast<u64>(1.0) : 0uLL;
				words = Ops { static_cast<u32>(b), static_cast<u32>(b >> 32uLL) };
			} else if (kind_ == ScalarKind::eFloat) {
				words = Ops { one_ ? (bits_ == 16uL ? 0x3C00uL : std::bit_cast<u32>(1.0F)) : 0uL };
			} else {
				words = Ops { one_ ? 1uL : 0uL };
				if (bits_ == 64uL) {
					words.emplace_back(0uL);
				}
			}
			c = w.constant(spv::Op::OpConstant, scalar(kind_, bits_), words);
		}
		return n_ <= 1uL ? c : w.constant(spv::Op::OpConstantComposite, shaped(kind_, bits_, n_), Ops(n_, c));
	}
}
