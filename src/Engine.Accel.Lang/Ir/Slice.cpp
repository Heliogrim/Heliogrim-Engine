#include "Slice.hpp"

#include <algorithm>
#include <utility>

#include "Compact.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	constexpr u32 unset = ~0uL;

	template <typename Type_>
	void insertSorted(ref<Vector<Type_>> list_, ref<const Type_> value_) {
		const auto it = std::ranges::lower_bound(list_, value_);
		if (it == list_.end() || *it != value_) {
			list_.insert(it, value_);
		}
	}

	enum class Role : u8 {
		/* always live: writes a kept target, returns, or is a body root */
		eLive,
		/* writes a dropped output */
		eDead,
		/* defines `Facts::defines`: live if that local is read */
		eLocal,
		/* expression statement: live only if a callee may write */
		eCall,
		/* block, if, when: live if a child is */
		eContainer
	};

	/* what one statement's own expressions touch (children excluded) */
	struct Facts {
		Vector<String> reads;
		Vector<u32> calls;
		Vector<u32> bindings;
		String defines;
	};

	struct Slicer {
		ref<const ir::Module> in;
		/* per binding: an output that is not required */
		ref<const Vector<bool>> dropped;

		Vector<Facts> facts = Vector<Facts>(in.stmts.size());
		Vector<Role> roles = Vector<Role>(in.stmts.size(), Role::eLive);
		Vector<bool> seen = Vector<bool>(in.stmts.size(), false);
		/* `&mut` params and aliases of the function being collected: writes through them are visible outside */
		Vector<String> mutRefs {};
		Vector<bool> live = Vector<bool>(in.stmts.size(), false);
		/* per function: its statements in pre-order, the root first */
		Vector<Vector<u32>> bodies = Vector<Vector<u32>>(in.functions.size());
		/* per function: may write something other than its locals and dropped outputs */
		Vector<bool> impure = Vector<bool>(in.functions.size(), false);

		/* like usageOf: resolved index, else by local name */
		[[nodiscard]] u32 binding(ref<const ir::Expr> expr_) const {
			if (expr_.local || expr_.binding.index < in.bindings.size()) {
				return expr_.local ? unset : expr_.binding.index;
			}
			if (expr_.binding.index != unset) {
				return unset;
			}
			const auto it = std::ranges::find(in.bindings, expr_.name, &ir::BindingDecl::localName);
			return it == in.bindings.end() ? unset : static_cast<u32>(it - in.bindings.begin());
		}

		[[nodiscard]] u32 function(ref<const ir::Expr> expr_) const {
			if (expr_.function.index < in.functions.size()) {
				return expr_.function.index;
			}
			if (std::ranges::count(in.functions, expr_.name, &ir::FunctionDecl::name) != 1) {
				return unset;
			}
			const auto it = std::ranges::find(in.functions, expr_.name, &ir::FunctionDecl::name);
			return it == in.functions.end() ? unset : static_cast<u32>(it - in.functions.begin());
		}

		void gather(ir::ExprRef root_, ref<Facts> facts_) const {
			Vector<u32> work {};
			if (root_.index < in.exprs.size()) {
				work.emplace_back(root_.index);
			}
			// a tree has fewer nodes than the arena; the budget stops hand-built cycles
			for (auto budget = in.exprs.size(); not work.empty() && budget > 0uz; --budget) {
				const auto& e = in.exprs[work.back()];
				work.pop_back();
				if (e.kind == ir::ExprKind::eNameRef && e.local) {
					insertSorted(facts_.reads, e.name);
				} else if (e.kind == ir::ExprKind::eNameRef) {
					if (const auto b = binding(e); b != unset) {
						insertSorted(facts_.bindings, b);
					}
				} else if (e.kind == ir::ExprKind::eCall) {
					insertSorted(facts_.calls, function(e));
				}
				for (const auto& op : e.operands) {
					if (op.index < in.exprs.size()) {
						work.emplace_back(op.index);
					}
				}
			}
		}

		/* the variable an lvalue writes: `a.x` and `a[i]` write `a` */
		[[nodiscard]] u32 base(ir::ExprRef ref_) const {
			auto at = ref_.index;
			for (auto steps = 0uz; at < in.exprs.size() && steps <= in.exprs.size(); ++steps) {
				const auto& e = in.exprs[at];
				if (e.kind != ir::ExprKind::eField && e.kind != ir::ExprKind::eIndex) {
					return at;
				}
				at = e.operands.empty() ? unset : e.operands.front().index;
			}
			return unset;
		}

		void classify(u32 stmt_) {
			const auto& s = in.stmts[stmt_];
			auto& f = facts[stmt_];
			gather(s.lhs, f);
			gather(s.rhs, f);
			// return stays eLive
			if (s.kind == ir::StmtKind::eAssign) {
				const auto at = base(s.lhs);
				const auto* target = at == unset ? nullptr : &in.exprs[at];
				if (target != nullptr && target->kind == ir::ExprKind::eNameRef && target->local &&
					std::ranges::find(mutRefs, target->name) != mutRefs.end()) {
					roles[stmt_] = Role::eLive;// writes the referenced place, not a local of its own
				} else if (target != nullptr && target->kind == ir::ExprKind::eNameRef && target->local) {
					roles[stmt_] = Role::eLocal;
					f.defines = target->name;
				} else if (target != nullptr && target->kind == ir::ExprKind::eNameRef) {
					const auto b = binding(*target);
					roles[stmt_] = b != unset && dropped[b] ? Role::eDead : Role::eLive;
				}
			} else if (s.kind == ir::StmtKind::eDeclareLocal || s.kind == ir::StmtKind::eDeclareRef) {
				if (s.kind == ir::StmtKind::eDeclareRef && s.mutable_) {
					mutRefs.emplace_back(s.name);
				}
				roles[stmt_] = Role::eLocal;
				f.defines = s.name;
			} else if (s.kind == ir::StmtKind::eExpr) {
				roles[stmt_] = Role::eCall;
			} else if (s.kind == ir::StmtKind::eBlock || s.kind == ir::StmtKind::eWhen || s.kind == ir::StmtKind::eIf) {
				roles[stmt_] = Role::eContainer;
			}
		}

		void collect(u32 function_) {
			const auto& fn = in.functions[function_];
			if (not fn.body.has_value()) {
				impure[function_] = true;// provided elsewhere: assume it writes
				return;
			}
			mutRefs.clear();
			for (const auto& param : fn.params) {
				if (param.kind == ir::ParamKind::eRefMut) {
					mutRefs.emplace_back(param.name);
				}
			}
			Vector<u32> work { fn.body->root.index };
			while (not work.empty()) {
				const auto at = work.back();
				work.pop_back();
				if (at >= in.stmts.size() || seen[at]) {
					continue;
				}
				seen[at] = true;
				bodies[function_].emplace_back(at);
				classify(at);
				const auto& children = in.stmts[at].children;
				for (auto i = children.size(); i > 0uz; --i) {
					work.emplace_back(children[i - 1uz].index);
				}
			}
			if (not bodies[function_].empty()) {
				roles[bodies[function_].front()] = Role::eLive;
			}
		}

		[[nodiscard]] bool mayWrite(u32 stmt_) const {
			return std::ranges::any_of(facts[stmt_].calls, [&](u32 c_) { return c_ == unset || impure[c_]; });
		}

		void purity() {
			for (auto f = 0uz; f < bodies.size(); ++f) {
				impure[f] = impure[f] || std::ranges::any_of(in.functions[f].params, [](const auto& p_) { return p_.kind == ir::ParamKind::eRefMut; }) || std::ranges::any_of(
					bodies[f],
					[&](u32 s_) { return roles[s_] == Role::eLive && in.stmts[s_].kind == ir::StmtKind::eAssign; }
				);
			}
			for (auto changed = true; changed;) {
				changed = false;
				for (auto f = 0uz; f < bodies.size(); ++f) {
					if (not impure[f] && std::ranges::any_of(bodies[f], [&](u32 s_) { return mayWrite(s_); })) {
						impure[f] = changed = true;
					}
				}
			}
		}

		void liveness(u32 function_) {
			const auto& list = bodies[function_];
			Vector<String> names {};
			const auto mark = [&](u32 s_) {
				live[s_] = true;
				for (const auto& name : facts[s_].reads) {
					insertSorted(names, name);
				}
			};
			for (const auto s : list) {
				// any statement whose expressions call a writer stays, even a write to a dropped output or a condition
				if (roles[s] == Role::eLive || mayWrite(s)) {
					mark(s);
				}
			}
			for (auto changed = true; changed;) {
				changed = false;
				// reverse pre-order: children before their parents
				for (auto i = list.size(); i > 0uz; --i) {
					const auto s = list[i - 1uz];
					if (live[s]) {
						continue;
					}
					const auto& children = in.stmts[s].children;
					const auto read = roles[s] == Role::eLocal && std::ranges::binary_search(names, facts[s].defines);
					const auto used = roles[s] == Role::eContainer &&
						std::ranges::any_of(children, [&](ir::StmtRef c_) { return c_.index < live.size() && live[c_.index]; });
					if (read || used) {
						mark(s);
						changed = true;
					}
				}
			}
		}
	};
}

ir::Module ir::slice(ref<const Module> module_, std::span<const String> requiredOutputIds_) {
	if (requiredOutputIds_.empty()) {
		return module_;
	}

	const auto& bindings = module_.bindings;
	Vector<bool> dropped(bindings.size(), false);
	Vector<bool> keep(bindings.size(), false);
	for (auto i = 0uz; i < bindings.size(); ++i) {
		const auto required = std::ranges::find(requiredOutputIds_, bindings[i].id) != requiredOutputIds_.end();
		dropped[i] = not bindings[i].input && not required;
		keep[i] = not bindings[i].input && required;
	}

	CompactPlan plan {};
	const auto main = std::ranges::find_if(
		module_.functions,
		[](ref<const FunctionDecl> fn_) { return fn_.name == "main" && fn_.body.has_value(); }
	);
	if (main != module_.functions.end()) {
		Slicer slicer { module_, dropped };
		for (auto f = 0uz; f < module_.functions.size(); ++f) {
			slicer.collect(static_cast<u32>(f));
		}
		slicer.purity();

		Vector<bool> kept(module_.functions.size(), false);
		Vector<u32> work { static_cast<u32>(main - module_.functions.begin()) };
		kept[work.front()] = true;
		while (not work.empty()) {
			const auto f = work.back();
			work.pop_back();
			slicer.liveness(f);
			for (const auto s : slicer.bodies[f]) {
				if (not slicer.live[s]) {
					continue;
				}
				for (const auto callee : slicer.facts[s].calls) {
					if (callee != unset && not kept[callee]) {
						kept[callee] = true;
						work.emplace_back(callee);
					}
				}
				/* shortcut: a live read of a dropped output, or a write to it kept for a callee that writes a kept
				 * target (`color = Shade();`), keeps that binding (a read may see undefined), upgrade when Task 11
				 * contract checks report it */
				for (const auto b : slicer.facts[s].bindings) {
					keep[b] = true;
				}
			}
		}

		for (auto f = 0uz; f < kept.size(); ++f) {
			if (kept[f]) {
				plan.functions.emplace_back(static_cast<u32>(f));
			}
		}
		plan.stmtAlias.resize(module_.stmts.size());
		for (auto s = 0uz; s < module_.stmts.size(); ++s) {
			plan.stmtAlias[s] = slicer.live[s] ? static_cast<u32>(s) : unset;
		}
	}

	for (auto i = 0uz; i < bindings.size(); ++i) {
		plan.bindingUses.emplace_back(static_cast<u32>(i));
		if (keep[i]) {
			plan.bindings.emplace_back(static_cast<u32>(i));
		}
	}
	return compact(module_, plan).module;
}
