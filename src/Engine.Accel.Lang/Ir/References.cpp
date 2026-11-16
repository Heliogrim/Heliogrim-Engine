#include "References.hpp"

#include <algorithm>
#include <cassert>
#include <variant>

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	using namespace hg::engine::accel::lang::ir;

	/* key text of a literal index by value, so `[0]` and `[0u]` share one key */
	String valueKey(ref<const LiteralValue> literal_) {
		return std::visit([](const auto value_) { return std::to_string(value_); }, literal_.value);
	}

	[[nodiscard]] bool takesReference(ref<const Module> m_, ref<const FunctionDecl> fn_) {
		return fn_.body.has_value() && std::ranges::any_of(
			fn_.params,
			[&](const auto& param_) {
				return param_.kind != ParamKind::eValue
					|| (param_.type.index < m_.types.size() && m_.types[param_.type.index].kind == TypeKind::eRegion);
			}
		);
	}

	/* scope entry; a region (`region`), a reference alias (`place` set) or, with neither, a local shadowing an outer entry */
	struct Region {
		String name;
		ExprRef root;
		ExprRef off;
		ExprRef len;
		bool region = false;
		/* root's base name (`scratch` in `scratch[..]`); a declaration of it would rebind the root at later uses */
		String rootName;
		/* reference alias: the lowered place every use of `name` becomes */
		ExprRef place;
	};

	/* reference argument as a place: root + access path, runtime indices in path order */
	struct Path {
		bool global = false;
		String key;
		ExprRef root;
		Vector<ExprRef> dynamic;
	};

	/* specialization worklist: one per (function, shape of each reference argument) */
	struct Specs {
		struct Pending {
			u32 function;
			Vector<Region> scope;
		};

		Vector<std::pair<String, u32>> keys;
		Vector<Pending> pending;
	};

	/* rewrites one function body; the initial `scope` binds region names to (root, off, len) */
	struct Rewriter {
		ref<Module> m;
		ref<Diagnostics> diags;
		Vector<Region> scope;
		ref<Specs> specs;

		/* nodes are copied before appending: the arenas reallocate */
		ExprRef push(mref<Expr> expr_) {
			m.exprs.emplace_back(::hg::move(expr_));
			return ExprRef { static_cast<u32>(m.exprs.size() - 1uL) };
		}

		StmtRef push(Stmt stmt_) {
			m.stmts.emplace_back(::hg::move(stmt_));
			return StmtRef { static_cast<u32>(m.stmts.size() - 1uL) };
		}

		ExprRef binary(ExprRef lhs_, Op op_, ExprRef rhs_, TypeRef type_, SourceLoc loc_) {
			return push(Expr { .kind = ExprKind::eBinary, .op = op_, .type = type_, .operands = { lhs_, rhs_ }, .loc = loc_ });
		}

		ExprRef nameref(String name_, TypeRef type_, SourceLoc loc_) {
			return push(Expr { .kind = ExprKind::eNameRef, .name = ::hg::move(name_), .local = true, .type = type_, .loc = loc_ });
		}

		[[nodiscard]] bool isRegion(TypeRef type_) const {
			return type_.index < m.types.size() && m.types[type_.index].kind == TypeKind::eRegion;
		}

		/* hoists every runtime index of the root place into a `const` (evaluated once, at the borrow); returns the base name */
		String freeze(ExprRef ref_, StringView name_, ref<Vector<StmtRef>> hoists_) {
			if (ref_.index >= m.exprs.size()) {
				return {};
			}

			const auto e = m.exprs[ref_.index];
			if (e.kind == ExprKind::eNameRef) {
				return e.name;
			}

			if ((e.kind != ExprKind::eField && e.kind != ExprKind::eIndex) || e.operands.empty()) {
				return {};
			}

			auto base = freeze(e.operands[0], name_, hoists_);
			if (e.kind == ExprKind::eIndex && e.operands.size() == 2uL && e.operands[1].index < m.exprs.size()
				&& m.exprs[e.operands[1].index].kind != ExprKind::eLiteral) {
				const auto type = m.exprs[e.operands[1].index].type;
				auto hoisted = String { name_ } + "__i" + std::to_string(hoists_.size());
				hoists_.emplace_back(
					push(
						Stmt {
							.kind = StmtKind::eDeclareLocal, .name = hoisted, .type = type, .lhs = e.operands[1], .mutable_ = false,
							.loc = e.loc
						}
					)
				);

				const auto use = nameref(::hg::move(hoisted), type, e.loc);
				m.exprs[ref_.index].operands[1] = use;
			}

			return base;
		}

		/* range: `len` holds `hi` until the borrow names its offset; `hoists_` set = freeze a fresh root */
		Opt<Region> regionOf(ExprRef ref_, StringView name_ = {}, Vector<StmtRef>* hoists_ = nullptr) {
			if (ref_.index >= m.exprs.size()) {
				return None;
			}

			const auto e = m.exprs[ref_.index];
			if (e.kind == ExprKind::eNameRef && e.local) {
				for (auto it = scope.rbegin(); it != scope.rend(); ++it) {
					if (it->name == e.name) {
						return it->region ? SomeVal<Region>(*it) : None;
					}
				}
				return None;
			}

			if (e.kind != ExprKind::eRange || e.operands.size() != 3uL) {
				return None;
			}

			const auto lo = rewrite(e.operands[1]);
			const auto hi = rewrite(e.operands[2]);
			const auto u32Type = m.exprs[lo.index].type;
			Region r { .region = true };
			if (auto base = regionOf(e.operands[0])) {
				r.root = base->root;
				r.rootName = base->rootName;
				r.off = binary(base->off, Op::eAdd, lo, u32Type, e.loc);
				r.len = binary(base->off, Op::eAdd, hi, u32Type, e.loc);

			} else {
				r.root = rewrite(e.operands[0]);
				if (hoists_ != nullptr) {
					r.rootName = freeze(r.root, name_, *hoists_);
				}
				r.off = lo;
				r.len = hi;
			}

			// the range node becomes the offset, so no eRange survives
			auto off = m.exprs[r.off.index];
			off.loc = e.loc;
			m.exprs[ref_.index] = ::hg::move(off);
			r.off = ref_;

			return SomeVal(::hg::move(r));
		}

		TypeRef u32Type() {
			for (auto i = 0uz; i < m.types.size(); ++i) {
				const auto& t = m.types[i];
				if (t.kind == TypeKind::eScalar && t.scalar == ScalarKind::eUInt && t.bits == 32uL) {
					return TypeRef { static_cast<u32>(i) };
				}
			}

			m.types.emplace_back(TypeDesc { .scalar = ScalarKind::eUInt });
			return TypeRef { static_cast<u32>(m.types.size() - 1uL) };
		}

		ExprRef clone(const ExprRef ref_) {
			if (ref_.index >= m.exprs.size()) {
				return ref_;
			}

			auto e = m.exprs[ref_.index];
			for (auto& operand : e.operands) {
				operand = clone(operand);
			}
			return push(::hg::move(e));
		}

		StmtRef clone(const StmtRef ref_) {
			if (ref_.index >= m.stmts.size()) {
				return ref_;
			}

			auto s = m.stmts[ref_.index];
			s.lhs = clone(s.lhs);
			s.rhs = clone(s.rhs);
			for (auto& child : s.children) {
				child = clone(child);
			}

			return push(::hg::move(s));
		}

		/* key text of a static root (binding name, fields, literal indices); empty = local or runtime-indexed */
		String place(const ExprRef ref_, ref<String> base_) const {
			if (ref_.index >= m.exprs.size()) {
				return {};
			}

			const auto& e = m.exprs[ref_.index];
			if (e.kind == ExprKind::eNameRef) {
				if (e.local || e.binding.index >= m.bindings.size()) {
					return {};
				}
				base_ = e.name;
				return e.name;
			}

			if (e.operands.empty()) {
				return {};
			}

			auto path = place(e.operands[0], base_);
			if (path.empty()) {
				return {};
			}

			if (e.kind == ExprKind::eField) {
				return path + "." + e.name;
			}

			if (e.kind == ExprKind::eIndex && e.operands.size() == 2uL && e.operands[1].index < m.exprs.size()
				&& m.exprs[e.operands[1].index].kind == ExprKind::eLiteral) {
				return path + "[" + valueKey(m.exprs[e.operands[1].index].literal) + "]";
			}

			return {};
		}

		/* a lowered place as root + path; none for anything but a name/builtin followed by fields and indices */
		Opt<Path> pathOf(const ExprRef ref_) const {
			if (ref_.index >= m.exprs.size()) {
				return None;
			}

			const auto& e = m.exprs[ref_.index];
			if (e.kind == ExprKind::eNameRef) {
				return e.local ?
					SomeVal(Path { .key = "L:" + std::to_string(e.type.index), .root = ref_ }) :
					SomeVal(Path { .global = true, .key = "G:" + e.name, .root = ref_ });
			}

			if (e.kind == ExprKind::eBuiltin && e.operands.empty()) {
				return SomeVal(Path { .global = true, .key = "G:" + e.name, .root = ref_ });
			}

			if ((e.kind != ExprKind::eField && e.kind != ExprKind::eIndex) || e.operands.empty()) {
				return None;
			}

			auto path = pathOf(e.operands[0]);
			if (not path.has_value()) {
				return None;
			}

			if (e.kind == ExprKind::eField) {
				path->key += "." + e.name;
				return path;
			}

			if (e.operands.size() != 2uL || e.operands[1].index >= m.exprs.size()) {
				return None;
			}

			if (m.exprs[e.operands[1].index].kind == ExprKind::eLiteral) {
				path->key += "[" + valueKey(m.exprs[e.operands[1].index].literal) + "]";
			} else {
				path->key += "[*]";
				path->dynamic.emplace_back(e.operands[1]);
			}
			return path;
		}

		/* copy of the place `ref_` with the root replaced by `root_` (if set) and runtime index `k` by `<param_>__i<k>` */
		ExprRef rebase(const ExprRef ref_, const ExprRef root_, StringView param_, ref<u32> k_) {
			auto e = m.exprs[ref_.index];
			if (e.kind != ExprKind::eField && e.kind != ExprKind::eIndex) {
				return root_.index < m.exprs.size() ? root_ : clone(ref_);
			}

			e.operands[0] = rebase(e.operands[0], root_, param_, k_);
			if (e.kind == ExprKind::eIndex && m.exprs[e.operands[1].index].kind != ExprKind::eLiteral) {
				e.operands[1] = nameref(String { param_ } + "__i" + std::to_string(k_++), u32Type(), e.loc);
			}
			return push(::hg::move(e));
		}

		/* a region argument as (root, off value, end value); each bound is emitted once */
		Opt<Region> argument(const ExprRef ref_) {
			const auto range = ref_.index < m.exprs.size() && m.exprs[ref_.index].kind == ExprKind::eRange;
			if (auto r = regionOf(ref_)) {
				if (not range) {
					r->len = binary(r->off, Op::eAdd, r->len, u32Type(), m.exprs[ref_.index].loc);
				}
				return r;
			}

			// a sized array place passes whole
			const auto type = ref_.index < m.exprs.size() ? m.exprs[ref_.index].type : TypeRef {};
			if (type.index >= m.types.size() || m.types[type.index].kind != TypeKind::eArray || m.types[type.index].count == 0uL) {
				return None;
			}

			const auto count = m.types[type.index].count;
			const auto loc = m.exprs[ref_.index].loc;
			return SomeVal(
				Region {
					.root = rewrite(ref_),
					.off = push(Expr { .literal = LiteralValue { u64 { 0uLL }, "0u" }, .type = u32Type(), .loc = loc }),
					.len = push(Expr { .name = std::to_string(count) + "u", .type = u32Type(), .loc = loc }),
					.region = true
				}
			);
		}

		/* retargets a call with reference or region arguments to its specialization, queuing it on first use */
		void specialize(ExprRef ref_) {
			const auto e = m.exprs[ref_.index];
			const auto callee = m.functions[e.function.index];
			auto key = std::to_string(e.function.index);
			Vector<ExprRef> args {};
			Vector<Region> bound {};
			Vector<ParamDecl> params {};
			Vector<StmtRef> prologue {};
			for (auto i = 0uz; i < e.operands.size() && i < callee.params.size(); ++i) {
				const auto& param = callee.params[i];
				if (isRegion(param.type)) {
					auto r = argument(e.operands[i]);
					String rootName {};
					const auto path = r.has_value() ? place(r->root, rootName) : String {};
					if (path.empty()) {
						diags.emplace_back(
							Diagnostic::error(
								codes::asl("region-not-rooted"),
								"region argument to '" + callee.name + "' must be rooted in a buffer or workgroup array"
							).at(spanIn(m, e.loc))
						);
						return;
					}

					key += "|R:" + path;
					args.append_range(Vector<ExprRef> { r->off, r->len });
					bound.emplace_back(
						Region {
							.name = param.name, .root = r->root, .off = nameref(param.name + "__off", u32Type(), callee.loc),
							.len = nameref(param.name + "__len", u32Type(), callee.loc), .region = true, .rootName = ::hg::move(rootName)
						}
					);
					params.append_range(
						Vector<ParamDecl> {
							ParamDecl { .name = param.name + "__off", .type = u32Type() },
							ParamDecl { .name = param.name + "__end", .type = u32Type() }
						}
					);
					prologue.emplace_back(
						push(
							Stmt {
								.kind = StmtKind::eDeclareLocal, .name = param.name + "__len", .type = u32Type(),
								.lhs = binary(
									nameref(param.name + "__end", u32Type(), callee.loc),
									ir::Op::eSub,
									nameref(param.name + "__off", u32Type(), callee.loc),
									u32Type(),
									callee.loc
								),
								.mutable_ = false, .loc = callee.loc
							}
						)
					);
					continue;
				}

				const auto arg = rewrite(e.operands[i]);
				const auto path = param.kind != ParamKind::eValue ? pathOf(arg) : None;
				// typecheck admits only places to `&mut`; a non-place passed to `&` (e.g. a literal) goes by value
				assert(path.has_value() || param.kind != ParamKind::eRefMut);
				if (not path.has_value()) {
					key += "|v";
					args.emplace_back(arg);
					params.emplace_back(ParamDecl { .name = param.name, .type = param.type });
					continue;
				}

				key += "|" + path->key;
				u32 k = 0uL;
				ExprRef root {};
				if (not path->global) {
					// local root: the whole root passes by reference, the callee walks the path from it
					const auto rootType = m.exprs[path->root.index].type;
					args.emplace_back(path->root);
					params.emplace_back(ParamDecl { .name = param.name, .type = rootType, .kind = param.kind });
					root = nameref(param.name, rootType, callee.loc);
				}

				for (const auto index : path->dynamic) {
					// index params are u32 (design 7.3), so the key does not depend on the index type
					const auto ie = m.exprs[index.index];
					args.emplace_back(
						ie.type.index == u32Type().index ?
						index :
						push(Expr { .kind = ExprKind::eConstruct, .name = "u32", .type = u32Type(), .operands = { index }, .loc = ie.loc })
					);
					params.emplace_back(ParamDecl { .name = param.name + "__i" + std::to_string(k++), .type = u32Type() });
				}

				k = 0uL;
				const auto lowered = rebase(arg, root, param.name, k);
				bound.emplace_back(
					Region {
						.name = param.name,
						.rootName = path->global ? m.exprs[path->root.index].name : param.name,
						.place = lowered
					}
				);
			}

			auto known = std::ranges::find(specs.keys, key, &std::pair<String, u32>::first);
			if (known == specs.keys.end()) {
				// a value param named like a global root would capture it in the specialized body; reported once per specialisation
				for (const auto& param : params) {
					if (const auto hit = std::ranges::find(bound, param.name, &Region::rootName);
						hit != bound.end() && hit->name != param.name) {
						diags.emplace_back(
							Diagnostic::error(
								codes::asl("region-root-shadowed"),
								"'" + param.name + "' shadows the root of live " + (hit->region ? "region" : "reference") + " '" + hit->name + "'"
							).at(spanIn(m, callee.loc))
						);
					}
				}

				FunctionDecl spec { callee };
				spec.name = {};
				spec.origin = callee.name;
				spec.params = ::hg::move(params);

				const auto root = clone(callee.body->root);
				if (m.stmts[root.index].kind == StmtKind::eBlock) {
					auto& children = m.stmts[root.index].children;
					children.insert(children.begin(), prologue.begin(), prologue.end());

				} else {
					prologue.emplace_back(root);
				}

				spec.body = SomeVal<BodyRef>(
					m.stmts[root.index].kind == StmtKind::eBlock ?
					root :
					push(Stmt { .kind = StmtKind::eBlock, .children = ::hg::move(prologue) })
				);
				m.functions.emplace_back(::hg::move(spec));
				const auto index = static_cast<u32>(m.functions.size() - 1uL);
				specs.pending.emplace_back(Specs::Pending { index, ::hg::move(bound) });
				known = specs.keys.emplace(specs.keys.end(), ::hg::move(key), index);
			}

			auto& call = m.exprs[ref_.index];
			call.name = {};
			call.function = FunctionRef { known->second };
			call.operands = ::hg::move(args);
		}

		ExprRef rewrite(ExprRef ref_) {
			if (ref_.index >= m.exprs.size()) {
				return ref_;
			}

			const auto e = m.exprs[ref_.index];
			if (e.kind == ExprKind::eNameRef && e.local) {
				for (auto it = scope.rbegin(); it != scope.rend(); ++it) {
					if (it->name == e.name) {
						if (it->place.index < m.exprs.size()) {
							// the alias use becomes the full place; already lowered, so not rewritten again
							auto place = m.exprs[clone(it->place).index];
							place.loc = e.loc;
							m.exprs[ref_.index] = ::hg::move(place);
						}
						break;
					}
				}
				return ref_;
			}

			if (e.kind == ExprKind::eCall && e.function.index < m.functions.size() && takesReference(m, m.functions[e.function.index])) {
				specialize(ref_);
				return ref_;
			}

			if (e.kind == ExprKind::eIndex && e.operands.size() == 2uL) {
				if (auto base = regionOf(e.operands[0])) {
					const auto index = rewrite(e.operands[1]);
					const auto absolute = binary(base->off, Op::eAdd, index, m.exprs[base->off.index].type, e.loc);
					m.exprs[ref_.index].operands = { base->root, absolute };
					return ref_;
				}
			}

			if (e.kind == ExprKind::eBuiltin && e.name == "len" && e.operands.size() == 1uL && e.operands[0].index < m.exprs.size()
				&& m.exprs[e.operands[0].index].kind == ExprKind::eNameRef) {
				if (auto r = regionOf(e.operands[0])) {
					auto len = m.exprs[r->len.index];
					len.loc = e.loc;
					m.exprs[ref_.index] = ::hg::move(len);
					return ref_;
				}
			}

			if (e.kind == ExprKind::eRange) {
				return ref_;// region argument: specialized, not lowered here
			}

			for (const auto operand : e.operands) {
				rewrite(operand);
			}

			return ref_;
		}

		/* a local named like a live region's root would rebind the root in that region's lowered uses */
		void shadow(ref<const Stmt> s_) {
			for (const auto& live : scope) {
				if (not live.rootName.empty() && live.rootName == s_.name) {
					diags.emplace_back(
						Diagnostic::error(
							codes::asl("region-root-shadowed"),
							"'" + s_.name + "' shadows the root of live " + (live.region ? "region" : "reference") + " '" + live.name + "'"
						).at(spanIn(m, s_.loc))
					);
					break;
				}
			}
			scope.emplace_back(Region { .name = s_.name });
		}

		StmtRef arm(const StmtRef ref_) {
			const auto mark = scope.size();
			auto stmts = stmt(ref_);
			scope.resize(mark);
			return stmts.size() == 1uL ? stmts.front() : push(Stmt { .kind = StmtKind::eBlock, .children = ::hg::move(stmts) });
		}

		/* the rewritten statement plus the declarations it adds to the enclosing block */
		Vector<StmtRef> stmt(StmtRef ref_) {
			if (ref_.index >= m.stmts.size()) {
				return { ref_ };
			}

			const auto s = m.stmts[ref_.index];
			switch (s.kind) {
				case StmtKind::eBlock: {
					const auto mark = scope.size();
					Vector<StmtRef> children {};
					for (const auto child : s.children) {
						children.append_range(stmt(child));
					}

					scope.resize(mark);
					m.stmts[ref_.index].children = ::hg::move(children);
					return { ref_ };
				}
				case StmtKind::eDeclareLocal: {
					rewrite(s.lhs);
					shadow(s);
					return { ref_ };
				}
				case StmtKind::eDeclareRef: {
					if (not isRegion(s.type)) {
						// the alias disappears: runtime indices are frozen here, uses become the place
						Vector<StmtRef> hoists {};
						rewrite(s.lhs);
						auto rootName = freeze(s.lhs, s.name, hoists);
						shadow(s);
						scope.back().rootName = ::hg::move(rootName);
						scope.back().place = s.lhs;
						return hoists;
					}

					Vector<StmtRef> hoists {};
					// regionOf yields an end for a range, but a length for a whole region; it also overwrites the range node
					const auto whole = s.lhs.index < m.exprs.size() && m.exprs[s.lhs.index].kind != ExprKind::eRange;
					auto r = regionOf(s.lhs, s.name, &hoists);
					if (not r.has_value()) {
						diags.emplace_back(
							Diagnostic::error(
								codes::asl("internal-inconsistent-module"),
								"region argument was not rewritten before code generation"
							).at(spanIn(m, s.loc))
						);
						return { ref_ };
					}

					const auto u32Type = m.exprs[r->off.index].type; {
						auto& off = m.stmts[ref_.index];
						off.kind = StmtKind::eDeclareLocal;
						off.name = s.name + "__off";
						off.type = u32Type;
						off.lhs = r->off;
						off.rhs = {};
						off.mutable_ = false;
						off.children.clear();
					}
					r->off = nameref(s.name + "__off", u32Type, s.loc);
					const auto len = push(
						Stmt {
							.kind = StmtKind::eDeclareLocal, .name = s.name + "__len", .type = u32Type,
							.lhs = whole ? r->len : binary(r->len, Op::eSub, r->off, u32Type, s.loc), .mutable_ = false, .loc = s.loc
						}
					);
					r->name = s.name;
					r->len = nameref(s.name + "__len", u32Type, s.loc);
					scope.emplace_back(::hg::move(*r));
					hoists.append_range(Vector<StmtRef> { ref_, len });
					return hoists;
				}
				case StmtKind::eAssign: {
					rewrite(s.lhs);
					rewrite(s.rhs);
					return { ref_ };
				}
				case StmtKind::eExpr:
				case StmtKind::eReturn: {
					rewrite(s.lhs);
					return { ref_ };
				}
				case StmtKind::eIf:
				case StmtKind::eWhile:
				case StmtKind::eWhen: {
					rewrite(s.lhs);
					for (auto i = 0uz; i < s.children.size(); ++i) {
						const auto child = arm(s.children[i]);
						m.stmts[ref_.index].children[i] = child;
					}
					return { ref_ };
				}
				case StmtKind::eFor: {
					const auto mark = scope.size();
					Vector<StmtRef> before {};
					for (auto i = 0uz; i < s.children.size(); ++i) {
						auto lowered = stmt(s.children[i]);
						if (i == 0uz && lowered.size() > 1uL) {
							// a for header holds one declaration: the region consts go before the loop, the init stays empty
							before = ::hg::move(lowered);
							m.stmts[ref_.index].children[i] = push(Stmt { .kind = StmtKind::eBlock });

						} else {
							m.stmts[ref_.index].children[i] = lowered.size() == 1uL ?
								lowered.front() :
								push(Stmt { .kind = StmtKind::eBlock, .children = ::hg::move(lowered) });
						}

						if (i == 0uz) {
							rewrite(s.lhs);
						}
					}

					scope.resize(mark);
					if (before.empty()) {
						return { ref_ };
					}

					before.emplace_back(ref_);
					return { push(Stmt { .kind = StmtKind::eBlock, .children = ::hg::move(before), .loc = s.loc }) };
				}
				default: {
					return { ref_ };
				}
			}
		}
	};
}

std::expected<Module, Diagnostics> ir::lowerReferences(ref<const Module> mod_) {
	Module out { mod_ };
	Diagnostics diags {};
	Specs specs {};
	for (const auto& function : mod_.functions) {
		if (function.body.has_value() && not takesReference(mod_, function)) {
			Rewriter { out, diags, {}, specs }.stmt(function.body->root);
		}
	}

	// specialized bodies get the same rewrite with their params bound; the key dedupe ends the worklist
	for (auto i = 0uz; i < specs.pending.size(); ++i) {
		auto pending = specs.pending[i];
		const auto root = out.functions[pending.function].body->root;
		Rewriter { out, diags, ::hg::move(pending.scope), specs }.stmt(root);
	}

	if (not diags.empty()) {
		return std::unexpected(::hg::move(diags));
	}

	// originals taking references are dropped (specialisations may keep a local-root `&`); `function` indices are remapped
	Vector<u32> remap(out.functions.size(), ~0uL);
	Vector<FunctionDecl> kept {};
	for (auto i = 0uz; i < out.functions.size(); ++i) {
		if (i < mod_.functions.size() && takesReference(mod_, mod_.functions[i])) {
			continue;
		}
		remap[i] = static_cast<u32>(kept.size());
		kept.emplace_back(::hg::move(out.functions[i]));
	}

	out.functions = ::hg::move(kept);
	for (auto& e : out.exprs) {
		if (e.function.index < remap.size()) {
			e.function.index = remap[e.function.index];
		}
	}
	return out;
}
