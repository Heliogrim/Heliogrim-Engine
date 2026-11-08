#include "Analysis.hpp"

#include <algorithm>

#include "Builtin.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	constexpr auto unresolved = ~0uL;

	void insertSorted(Vector<u32>& list_, u32 value_) {
		const auto it = std::lower_bound(list_.begin(), list_.end(), value_);
		if (it == list_.end() || *it != value_) {
			list_.insert(it, value_);
		}
	}

	struct Walker {
		ref<const ir::Module> module;
		ir::Usage usage;

		void bindingAt(ref<const ir::Expr> expr_, Vector<u32>& into_) {
			if (expr_.local) {
				return;
			}

			auto index = expr_.binding.index;
			if (index == unresolved) {
				/* shortcut: unresolved names match BindingDecl::localName without scope shadowing, until the parser resolves them */
				const auto it = std::ranges::find(module.bindings, expr_.name, &ir::BindingDecl::localName);
				if (it == module.bindings.end()) {
					return;
				}
				index = static_cast<u32>(it - module.bindings.begin());
			}
			insertSorted(into_, index);
		}

		void expr(ir::ExprRef ref_, bool write_ = false) {
			if (ref_.index >= module.exprs.size()) {
				return;
			}

			const auto& e = module.exprs[ref_.index];
			switch (e.kind) {
				case ir::ExprKind::eNameRef: {
					bindingAt(e, write_ ? usage.writtenBindings : usage.readBindings);
					break;
				}
				case ir::ExprKind::eCall: {
					auto index = e.function.index;
					/* an ambiguous name is no call target; typecheck sets the ref whenever an overload is chosen */
					if (index == unresolved && std::ranges::count(module.functions, e.name, &ir::FunctionDecl::name) == 1) {
						const auto it = std::ranges::find(module.functions, e.name, &ir::FunctionDecl::name);
						index = it == module.functions.end() ? unresolved : static_cast<u32>(it - module.functions.begin());
					}
					if (index != unresolved) {
						insertSorted(usage.calledFunctions, index);
					}
					for (auto i = 0uz; i < e.operands.size(); ++i) {
						/* a `&mut` argument writes its root */
						const auto refMut = index < module.functions.size() && i < module.functions[index].params.size() &&
							module.functions[index].params[i].kind == ir::ParamKind::eRefMut;
						expr(e.operands[i], refMut);
					}
					break;
				}
				case ir::ExprKind::eBuiltin: {
					for (auto i = 0uz; i < e.operands.size(); ++i) {
						expr(e.operands[i], ir::builtinWritesArg(e.name, i));
					}
					break;
				}
				case ir::ExprKind::eField: {
					/* writing `a.x` writes `a` */
					for (const auto& op : e.operands) {
						expr(op, write_);
					}
					break;
				}
				case ir::ExprKind::eIndex: {
					/* writing `a[i]` writes `a`, reads `i` */
					for (auto i = 0uz; i < e.operands.size(); ++i) {
						expr(e.operands[i], write_ && i == 0uz);
					}
					break;
				}
				default: {
					for (const auto& op : e.operands) {
						expr(op);
					}
				}
			}
		}

		void stmt(ir::StmtRef ref_) {
			if (ref_.index >= module.stmts.size()) {
				return;
			}

			const auto& s = module.stmts[ref_.index];
			switch (s.kind) {
				case ir::StmtKind::eAssign: {
					expr(s.lhs, true);
					if (not s.name.empty()) {
						expr(s.lhs);// compound assignment reads its target too
					}
					expr(s.rhs);
					break;
				}
				case ir::StmtKind::eBlock:
				case ir::StmtKind::eWhen:
				case ir::StmtKind::eFor:
				case ir::StmtKind::eWhile:
				case ir::StmtKind::eIf: {
					expr(s.lhs);// if: the condition; unset (out of range) otherwise
					for (const auto& child : s.children) {
						stmt(child);
					}
					break;
				}
				case ir::StmtKind::eDeclareRef: {
					expr(s.lhs, s.mutable_);
					break;
				}
				default: {
					expr(s.lhs);
				}
			}
		}
	};
}

ir::Usage ir::usageOf(ref<const Module> module_, u32 functionIndex_) {
	Walker walker { module_, {} };
	if (functionIndex_ < module_.functions.size()) {
		const auto& body = module_.functions[functionIndex_].body;
		if (body.has_value()) {
			walker.stmt(body->root);
		}
	}
	return std::move(walker.usage);
}

Vector<u32> ir::reachableFunctions(ref<const Module> module_, std::span<const u32> roots_) {
	Vector<bool> seen(module_.functions.size(), false);
	Vector<u32> work {};
	for (const auto root : roots_) {
		if (root < seen.size() && !seen[root]) {
			seen[root] = true;
			work.emplace_back(root);
		}
	}

	while (!work.empty()) {
		const auto next = work.back();
		work.pop_back();
		for (const auto callee : usageOf(module_, next).calledFunctions) {
			if (callee < seen.size() && !seen[callee]) {
				seen[callee] = true;
				work.emplace_back(callee);
			}
		}
	}

	Vector<u32> result {};
	for (auto i = 0uz; i < seen.size(); ++i) {
		if (seen[i]) {
			result.emplace_back(static_cast<u32>(i));
		}
	}
	return result;
}
