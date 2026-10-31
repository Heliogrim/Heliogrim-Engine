#include "Analysis.hpp"

#include <algorithm>

using namespace hg;
using namespace hg::engine::accel::lang;

namespace {
	constexpr auto unresolved = ~0uL;

	void insertSorted(Vector<u32>& list_, u32 value_) {
		const auto it = std::lower_bound(list_.begin(), list_.end(), value_);
		if (it == list_.end() || *it != value_) {
			list_.insert(it, value_);
		}
	}

	struct Walker {
		cref<ir::Module> module;
		ir::Usage usage;

		void bindingAt(cref<ir::Expr> expr_, Vector<u32>& into_) {
			auto index = expr_.resolved;
			if (index == unresolved) {
				/* shortcut: no local-scope shadowing, names match BindingDecl::localName until the parser resolves them */
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
					auto index = e.resolved;
					if (index == unresolved) {
						const auto it = std::ranges::find(module.functions, e.name, &ir::FunctionDecl::name);
						index = it == module.functions.end() ? unresolved : static_cast<u32>(it - module.functions.begin());
					}
					if (index != unresolved) {
						insertSorted(usage.calledFunctions, index);
					}
					for (const auto& arg : e.operands) {
						expr(arg);
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
					expr(s.rhs);
					break;
				}
				case ir::StmtKind::eBlock:
				case ir::StmtKind::eWhen: {
					for (const auto& child : s.children) {
						stmt(child);
					}
					break;
				}
				default: {
					expr(s.lhs);
				}
			}
		}
	};
}

ir::Usage ir::usageOf(cref<Module> module_, u32 functionIndex_) {
	Walker walker { module_, {} };
	if (functionIndex_ < module_.functions.size()) {
		const auto& body = module_.functions[functionIndex_].body;
		if (body.has_value()) {
			walker.stmt(body->root);
		}
	}
	return std::move(walker.usage);
}

Vector<u32> ir::reachableFunctions(cref<Module> module_, std::span<const u32> roots_) {
	Vector<bool> seen(module_.functions.size(), false);
	Vector<u32> work {};
	for (const auto root : roots_) {
		if (root < seen.size() && !seen[root]) {
			seen[root] = true;
			work.push_back(root);
		}
	}

	while (!work.empty()) {
		const auto next = work.back();
		work.pop_back();
		for (const auto callee : usageOf(module_, next).calledFunctions) {
			if (callee < seen.size() && !seen[callee]) {
				seen[callee] = true;
				work.push_back(callee);
			}
		}
	}

	Vector<u32> result {};
	for (auto i = 0uz; i < seen.size(); ++i) {
		if (seen[i]) {
			result.push_back(static_cast<u32>(i));
		}
	}
	return result;
}
