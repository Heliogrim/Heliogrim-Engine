#include "Regions.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	using namespace hg::engine::accel::lang::ir;
	/* scope entry; `region = false` shadows an outer region of the same name */
	struct Region {
		String name;
		ExprRef root;
		ExprRef off;
		ExprRef len;
		bool region = false;
		/* root's base name (`scratch` in `scratch[..]`); a declaration of it would rebind the root at later uses */
		String rootName;
	};

	/* rewrites one function body; the initial `scope` binds region names to (root, off, len) */
	struct Rewriter {
		ref<Module> m;
		ref<Diagnostics> diags;
		Vector<Region> scope;

		/* nodes are copied before appending: the arenas reallocate */
		ExprRef push(Expr expr_) {
			m.exprs.emplace_back(std::move(expr_));
			return ExprRef { static_cast<u32>(m.exprs.size() - 1uL) };
		}

		StmtRef push(Stmt stmt_) {
			m.stmts.emplace_back(std::move(stmt_));
			return StmtRef { static_cast<u32>(m.stmts.size() - 1uL) };
		}

		ExprRef binary(ExprRef lhs_, StringView op_, ExprRef rhs_, TypeRef type_, SourceLoc loc_) {
			return push(Expr { .kind = ExprKind::eBinary, .name = String { op_ }, .type = type_, .operands = { lhs_, rhs_ }, .loc = loc_ });
		}

		ExprRef nameref(String name_, TypeRef type_, SourceLoc loc_) {
			return push(Expr { .kind = ExprKind::eNameRef, .name = std::move(name_), .local = true, .type = type_, .loc = loc_ });
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
					push(Stmt { .kind = StmtKind::eDeclareLocal, .name = hoisted, .type = type, .lhs = e.operands[1], .mutable_ = false, .loc = e.loc })
				);
				const auto use = nameref(std::move(hoisted), type, e.loc);
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
						return it->region ? Opt<Region> { *it } : None;
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
				r.off = binary(base->off, "+", lo, u32Type, e.loc);
				r.len = binary(base->off, "+", hi, u32Type, e.loc);
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
			m.exprs[ref_.index] = std::move(off);
			r.off = ref_;
			return r;
		}

		ExprRef rewrite(ExprRef ref_) {
			if (ref_.index >= m.exprs.size()) {
				return ref_;
			}
			const auto e = m.exprs[ref_.index];
			if (e.kind == ExprKind::eIndex && e.operands.size() == 2uL) {
				if (auto base = regionOf(e.operands[0])) {
					const auto index = rewrite(e.operands[1]);
					const auto absolute = binary(base->off, "+", index, m.exprs[base->off.index].type, e.loc);
					m.exprs[ref_.index].operands = { base->root, absolute };
					return ref_;
				}
			}
			if (e.kind == ExprKind::eBuiltin && e.name == "len" && e.operands.size() == 1uL && e.operands[0].index < m.exprs.size()
				&& m.exprs[e.operands[0].index].kind == ExprKind::eNameRef) {
				if (auto r = regionOf(e.operands[0])) {
					auto len = m.exprs[r->len.index];
					len.loc = e.loc;
					m.exprs[ref_.index] = std::move(len);
					return ref_;
				}
			}
			if (e.kind == ExprKind::eRange) {
				return ref_; // region argument: specialized, not lowered here
			}
			for (const auto operand : e.operands) {
				rewrite(operand);
			}
			return ref_;
		}

		/* a local named like a live region's root would rebind the root in that region's lowered uses */
		void shadow(ref<const Stmt> s_) {
			for (const auto& live : scope) {
				if (live.region && live.rootName == s_.name) {
					diags.emplace_back(
						Diagnostic {
							.message = "'" + s_.name + "' shadows the root of live region '" + live.name + "'", .line = s_.loc.line,
							.column = s_.loc.column
						}
					);
					break;
				}
			}
			scope.emplace_back(Region { .name = s_.name });
		}

		StmtRef arm(StmtRef ref_) {
			const auto mark = scope.size();
			auto stmts = stmt(ref_);
			scope.resize(mark);
			return stmts.size() == 1uL ? stmts.front() : push(Stmt { .kind = StmtKind::eBlock, .children = std::move(stmts) });
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
					m.stmts[ref_.index].children = std::move(children);
					return { ref_ };
				}
				case StmtKind::eDeclareLocal: {
					rewrite(s.lhs);
					shadow(s);
					return { ref_ };
				}
				case StmtKind::eDeclareRef: {
					if (not isRegion(s.type)) {
						rewrite(s.lhs);
						shadow(s);
						return { ref_ };
					}
					Vector<StmtRef> hoists {};
					// regionOf yields an end for a range, but a length for a whole region; it also overwrites the range node
					const auto whole = s.lhs.index < m.exprs.size() && m.exprs[s.lhs.index].kind != ExprKind::eRange;
					auto r = regionOf(s.lhs, s.name, &hoists);
					if (not r.has_value()) {
						diags.emplace_back(Diagnostic { .message = "region not lowered", .line = s.loc.line, .column = s.loc.column });
						return { ref_ };
					}
					const auto u32Type = m.exprs[r->off.index].type;
					{
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
							.lhs = whole ? r->len : binary(r->len, "-", r->off, u32Type, s.loc), .mutable_ = false, .loc = s.loc
						}
					);
					r->name = s.name;
					r->len = nameref(s.name + "__len", u32Type, s.loc);
					scope.emplace_back(std::move(*r));
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
							before = std::move(lowered);
							m.stmts[ref_.index].children[i] = push(Stmt { .kind = StmtKind::eBlock });
						} else {
							m.stmts[ref_.index].children[i] = lowered.size() == 1uL ? lowered.front() :
								push(Stmt { .kind = StmtKind::eBlock, .children = std::move(lowered) });
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
					return { push(Stmt { .kind = StmtKind::eBlock, .children = std::move(before), .loc = s.loc }) };
				}
				default: {
					return { ref_ };
				}
			}
		}
	};
}

std::expected<Module, Diagnostics> ir::lowerRegions(ref<const Module> module_) {
	Module out { module_ };
	Diagnostics diags {};
	for (const auto& function : module_.functions) {
		if (function.body.has_value()) {
			Rewriter { out, diags, {} }.stmt(function.body->root);
		}
	}
	if (not diags.empty()) {
		return std::unexpected(std::move(diags));
	}
	return out;
}
