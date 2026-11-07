#include "Compact.hpp"

#include <algorithm>
#include <utility>

#include "Typecheck.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	constexpr u32 unset = ~0uL;

	enum class Visit : u8 {
		eNew,
		eOpen,
		eDone
	};

	[[nodiscard]] u32 inRange(u32 index_, size_t limit_) {
		return index_ < limit_ ? index_ : unset;
	}

	[[nodiscard]] ir::TypeRef type(ir::TypeRef ref_, size_t types_) {
		return ir::TypeRef { inRange(ref_.index, types_) };
	}

	struct Compactor {
		ref<const ir::Module> in;
		ref<const ir::CompactPlan> plan;
		ir::Compacted out {};

		Vector<u32> newStmt = Vector<u32>(in.stmts.size(), unset);
		Vector<Visit> stmtVisit = Vector<Visit>(in.stmts.size(), Visit::eNew);
		Vector<u32> newExpr = Vector<u32>(in.exprs.size(), unset);
		Vector<Visit> exprVisit = Vector<Visit>(in.exprs.size(), Visit::eNew);
		Vector<u32> newFunction = Vector<u32>(in.functions.size(), unset);
		Vector<u32> newBinding = Vector<u32>(in.bindings.size(), unset);
		Vector<u32> newConstant = Vector<u32>(in.constants.size(), unset);

		/* follows inline aliases; ~0 if dropped, out of range or an alias cycle */
		[[nodiscard]] u32 target(u32 stmt_) const {
			for (auto steps = 0uz; stmt_ < in.stmts.size() && steps <= in.stmts.size(); ++steps) {
				const auto next = stmt_ < plan.stmtAlias.size() ? plan.stmtAlias[stmt_] : stmt_;
				if (next == stmt_) {
					return stmt_;
				}
				stmt_ = next;
			}
			return unset;
		}

		[[nodiscard]] u32 emptyBlock() {
			out.module.stmts.emplace_back().kind = ir::StmtKind::eBlock;
			return static_cast<u32>(out.module.stmts.size() - 1uz);
		}

		/* pre-order copy; children hold input indices (~0 = dropped or back edge) until fixStmts */
		[[nodiscard]] u32 body(ir::StmtRef root_) {
			const auto root = target(root_.index);
			if (root == unset) {
				return emptyBlock();
			}
			Vector<std::pair<u32, bool>> work { { root, false } };
			while (not work.empty()) {
				const auto [at, leave] = work.back();
				work.pop_back();
				if (leave) {
					stmtVisit[at] = Visit::eDone;
					continue;
				}
				if (stmtVisit[at] != Visit::eNew) {
					continue;// shared with an earlier parent
				}
				stmtVisit[at] = Visit::eOpen;
				newStmt[at] = static_cast<u32>(out.module.stmts.size());

				auto copy = in.stmts[at];
				work.emplace_back(at, true);
				for (auto& child : copy.children) {
					child.index = target(child.index);
					if (child.index != unset && stmtVisit[child.index] == Visit::eOpen) {
						child.index = unset;// an ancestor: cut the cycle
					}
				}
				for (auto i = copy.children.size(); i > 0uz; --i) {
					if (copy.children[i - 1uz].index != unset) {
						work.emplace_back(copy.children[i - 1uz].index, false);
					}
				}
				out.module.stmts.emplace_back(std::move(copy));
			}
			return newStmt[root];
		}

		void fixStmts() {
			for (auto i = 0uz; i < out.module.stmts.size(); ++i) {
				auto children = std::move(out.module.stmts[i].children);
				const auto block = out.module.stmts[i].kind == ir::StmtKind::eBlock;
				Vector<ir::StmtRef> mapped {};
				for (const auto child : children) {
					if (child.index != unset) {
						mapped.emplace_back(ir::StmtRef { newStmt[child.index] });
					} else if (not block) {
						mapped.emplace_back(ir::StmtRef { emptyBlock() });// keeps `if` arms in place
					}
				}
				out.module.stmts[i].children = std::move(mapped);
			}
		}

		/* pre-order copy; operands hold input indices until fixExprs */
		[[nodiscard]] ir::ExprRef expr(ir::ExprRef root_) {
			if (root_.index >= in.exprs.size()) {
				return {};
			}
			Vector<std::pair<u32, bool>> work { { root_.index, false } };
			while (not work.empty()) {
				const auto [at, leave] = work.back();
				work.pop_back();
				if (leave) {
					exprVisit[at] = Visit::eDone;
					continue;
				}
				if (exprVisit[at] != Visit::eNew) {
					continue;
				}
				exprVisit[at] = Visit::eOpen;
				newExpr[at] = static_cast<u32>(out.module.exprs.size());

				auto copy = in.exprs[at];
				work.emplace_back(at, true);
				for (auto& op : copy.operands) {
					op.index = inRange(op.index, in.exprs.size());
					if (op.index != unset && exprVisit[op.index] == Visit::eOpen) {
						op.index = unset;
					}
				}
				for (auto i = copy.operands.size(); i > 0uz; --i) {
					if (copy.operands[i - 1uz].index != unset) {
						work.emplace_back(copy.operands[i - 1uz].index, false);
					}
				}
				out.module.exprs.emplace_back(std::move(copy));
			}
			return ir::ExprRef { newExpr[root_.index] };
		}

		[[nodiscard]] ir::FunctionRef call(ref<const ir::Expr> expr_) {
			const auto old = expr_.function.index;
			if (old < newFunction.size() && newFunction[old] != unset) {
				return ir::FunctionRef { newFunction[old] };
			}
			/* dropped or unset ref: a kept function with the same signature (a name alone picks the wrong overload) */
			const auto known = old < in.functions.size();
			const auto wanted = known ? signatureOf(in, in.functions[old]) : expr_.name;
			u32 found = unset;
			auto count = 0uz;
			for (auto i = 0uz; i < newFunction.size(); ++i) {
				if (newFunction[i] != unset && in.functions[i].name == expr_.name && (not known || signatureOf(in, in.functions[i]) == wanted)) {
					found = newFunction[i];
					++count;
				}
			}
			if (count == 1uz || (known && count > 1uz)) {
				return ir::FunctionRef { found };
			}
			if (std::ranges::find(out.unresolvedCalls, wanted) == out.unresolvedCalls.end()) {
				out.unresolvedCalls.emplace_back(wanted);
			}
			return {};
		}

		void fixExprs() {
			const auto types = out.module.types.size();
			for (auto& e : out.module.exprs) {
				for (auto& op : e.operands) {
					op.index = op.index == unset ? unset : newExpr[op.index];
				}
				const auto old = e.binding.index;
				const auto uses = old < plan.bindingUses.size() ? plan.bindingUses[old] : unset;
				e.binding.index = e.local || uses >= newBinding.size() ? unset : newBinding[uses];
				e.constant.index = e.local || e.constant.index >= newConstant.size() ? unset : newConstant[e.constant.index];
				e.function = e.kind == ir::ExprKind::eCall ? call(e) : ir::FunctionRef {};
				e.type = type(e.type, types);
			}
		}

		void run() {
			auto& result = out.module;
			result.name = in.name;
			result.entry = in.entry;
			result.toggles = in.toggles;
			result.types = in.types;
			const auto types = result.types.size();
			for (auto& t : result.types) {
				t.element = type(t.element, types);
				for (auto& field : t.fields) {
					field.type = type(field.type, types);
				}
			}

			for (const auto old : plan.bindings) {
				if (old < in.bindings.size()) {
					newBinding[old] = static_cast<u32>(result.bindings.size());
					auto& binding = result.bindings.emplace_back(in.bindings[old]);
					binding.type = type(binding.type, types);
				}
			}
			for (auto old = 0uz; old < in.constants.size(); ++old) {
				if (plan.constants.has_value() && not std::ranges::binary_search(*plan.constants, static_cast<u32>(old))) {
					continue;
				}
				newConstant[old] = static_cast<u32>(result.constants.size());
				auto& constant = result.constants.emplace_back(in.constants[old]);
				constant.type = type(constant.type, types);
			}
			for (const auto old : plan.functions) {
				if (old < in.functions.size()) {
					newFunction[old] = static_cast<u32>(result.functions.size());
					result.functions.emplace_back(in.functions[old]);
				}
			}

			for (auto& fn : result.functions) {
				for (auto& param : fn.params) {
					param.type = type(param.type, types);
				}
				fn.ret = type(fn.ret, types);
				if (fn.body.has_value()) {
					fn.body->root = ir::StmtRef { body(fn.body->root) };
				}
			}
			fixStmts();

			for (auto i = 0uz; i < result.stmts.size(); ++i) {
				result.stmts[i].type = type(result.stmts[i].type, types);
				result.stmts[i].lhs = expr(result.stmts[i].lhs);
				result.stmts[i].rhs = expr(result.stmts[i].rhs);
			}
			for (auto& constant : result.constants) {
				constant.value = expr(constant.value);
			}
			fixExprs();
		}
	};
}

ir::Compacted ir::compact(ref<const Module> module_, ref<const CompactPlan> plan_) {
	Compactor compactor { module_, plan_ };
	compactor.run();
	return std::move(compactor.out);
}
