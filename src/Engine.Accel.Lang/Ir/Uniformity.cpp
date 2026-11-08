#include "Uniformity.hpp"

#include <map>

#include "Analysis.hpp"
#include "Builtin.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	struct FunctionWalker {
		ref<const ir::Module> mod;
		FnRef<bool(StringView)> needsUniform;
		ir::Uniformity& out;

		/* shortcut: locals keyed by name without scope shadowing; a shadowing redeclaration merges with the outer one (conservative) */
		std::map<String, bool> divergent {};
		/* `&mut` alias name -> root local */
		std::map<String, String> alias {};
		bool changed = false;
		bool direct = false;
		/* sticky taint of a divergent break/continue in the current loop / return in the function */
		bool loopTaint = false;
		bool fnTaint = false;

		void mark(const String& name_, bool lane_) {
			auto& slot = divergent[name_];
			if (lane_ && not slot) {
				slot = true;
				changed = true;
			}
		}

		[[nodiscard]] bool exprUniform(ir::ExprRef ref_) {
			if (ref_.index >= mod.exprs.size()) {
				return true;
			}

			const auto& e = mod.exprs[ref_.index];
			bool uniform = true;
			for (const auto op : e.operands) {
				uniform = exprUniform(op) && uniform;
			}

			switch (e.kind) {
				case ir::ExprKind::eLiteral: {
					return true;
				}
				case ir::ExprKind::eNameRef: {
					if (e.local) {
						const auto it = divergent.find(e.name);
						return it != divergent.end() && not it->second;
					}
					if (e.constant.index < mod.constants.size()) {
							return true;
						}
						auto index = e.binding.index;
					if (index == ~0uL) {
						const auto it = std::ranges::find(mod.bindings, e.name, &ir::BindingDecl::localName);
						index = it == mod.bindings.end() ? ~0uL : static_cast<u32>(it - mod.bindings.begin());
					}
					if (index >= mod.bindings.size()) {
						return false;
					}
					/* storage reads are uniform when their index operands are (checked by the eIndex operands) */
					const auto cls = mod.bindings[index].bindClass;
					return cls == ir::BindClass::ePushConstant || cls == ir::BindClass::eUniformBuffer || cls == ir::BindClass::eStorageBuffer;
				}
				case ir::ExprKind::eBuiltin: {
					direct = direct || needsUniform(e.name);
					/* the `&mut` out argument of a builtin is filled with a lane-dependent result */
					for (auto i = 0uz; i < e.operands.size(); ++i) {
						if (ir::builtinWritesArg(e.name, i)) {
							assignTo(e.operands[i], true);
						}
					}
					const auto* variable = ir::findBuiltinVariable(e.name);
					return variable != nullptr && variable->uniform && uniform;
				}
				case ir::ExprKind::eCall: {
					/* shortcut: user call results are lane-dependent; upgrade with a return-uniformity pass */
					return false;
				}
				default: {
					/* field, index, unary, binary, construct, ternary: uniform iff all operands are */
					return uniform;
				}
			}
		}

		[[nodiscard]] bool divergentNow(bool cond_) const noexcept {
			return cond_ || loopTaint || fnTaint;
		}

		/* local a place is rooted at, aliases resolved to their root; empty when none */
		[[nodiscard]] String rootOf(ir::ExprRef place_) const {
			while (place_.index < mod.exprs.size()) {
				const auto& e = mod.exprs[place_.index];
				if ((e.kind == ir::ExprKind::eField || e.kind == ir::ExprKind::eIndex || e.kind == ir::ExprKind::eRange) && not e.operands.empty()) {
					place_ = e.operands.front();
					continue;
				}
				if (e.kind == ir::ExprKind::eNameRef && e.local) {
					const auto it = alias.find(e.name);
					return it != alias.end() ? it->second : e.name;
				}
				break;
			}
			return {};
		}

		void assignTo(ir::ExprRef place_, bool lane_) {
			const auto root = rootOf(place_);
			if (not root.empty()) {
				mark(root, lane_);
			}
			/* the written name itself too (an alias), so reads through it see the write */
			while (place_.index < mod.exprs.size()) {
				const auto& e = mod.exprs[place_.index];
				if ((e.kind == ir::ExprKind::eField || e.kind == ir::ExprKind::eIndex || e.kind == ir::ExprKind::eRange) && not e.operands.empty()) {
					place_ = e.operands.front();
					continue;
				}
				if (e.kind == ir::ExprKind::eNameRef && e.local) {
					mark(e.name, lane_);
				}
				return;
			}
		}

		void stmt(ir::StmtRef ref_, bool cond_) {
			if (ref_.index >= mod.stmts.size()) {
				return;
			}

			const auto& s = mod.stmts[ref_.index];
			out.uniformStmt[ref_.index] = not divergentNow(cond_);

			switch (s.kind) {
				case ir::StmtKind::eDeclareLocal: {
					/* evaluate first: builtin detection must run under divergent control too */
					const auto initLane = s.lhs.index < mod.exprs.size() && not exprUniform(s.lhs);
					mark(s.name, divergentNow(cond_) || initLane);
					break;
				}
				case ir::StmtKind::eDeclareRef: {
					mark(s.name, not exprUniform(s.lhs));
					const auto root = rootOf(s.lhs);
					if (not root.empty()) {
						alias[s.name] = root;
					}
					break;
				}
				case ir::StmtKind::eAssign: {
					const auto lane = not exprUniform(s.rhs) || divergentNow(cond_);
					assignTo(s.lhs, lane);
					(void)exprUniform(s.lhs);
					break;
				}
				case ir::StmtKind::eExpr: {
					(void)exprUniform(s.lhs);
					break;
				}
				case ir::StmtKind::eReturn: {
					(void)exprUniform(s.lhs);
					fnTaint = fnTaint || divergentNow(cond_);
					break;
				}
				case ir::StmtKind::eBreak:
				case ir::StmtKind::eContinue: {
					loopTaint = loopTaint || divergentNow(cond_);
					break;
				}
				case ir::StmtKind::eIf: {
					const auto lane = not exprUniform(s.lhs);
					for (const auto child : s.children) {
						stmt(child, cond_ || lane);
					}
					break;
				}
				case ir::StmtKind::eFor:
				case ir::StmtKind::eWhile: {
					loop(s, cond_);
					break;
				}
				default: {
					/* block, `when` (resolved before; remaining arms count as uniform control), discard */
					for (const auto child : s.children) {
						stmt(child, cond_);
					}
				}
			}
		}

		void loop(ref<const ir::Stmt> s_, bool cond_) {
			const auto outer = loopTaint;
			const auto fnBefore = fnTaint;
			const auto isFor = s_.kind == ir::StmtKind::eFor && s_.children.size() == 3uz;
			if (isFor) {
				stmt(s_.children[0], cond_);
			}

			/* a divergent break/continue taints the whole loop (condition and later iterations): rerun once with the taint set */
			auto taint = outer;
			for (auto pass = 0; pass < 2; ++pass) {
				loopTaint = taint;
				const auto lane = not exprUniform(s_.lhs) || (s_.lhs.index >= mod.exprs.size() && s_.kind == ir::StmtKind::eFor);
				if (isFor) {
					stmt(s_.children[1], cond_ || lane);
					stmt(s_.children[2], cond_ || lane);
				} else if (not s_.children.empty()) {
					stmt(s_.children.front(), cond_ || lane);
				}
				/* a divergent return in the body leaves the loop for some lanes only: same as a divergent break */
				if ((loopTaint && not taint) || (pass == 0 && fnTaint && not fnBefore)) {
					taint = true;
					continue;
				}
				break;
			}
			loopTaint = outer;
		}
	};
}

ir::Uniformity ir::analyzeUniformity(ref<const Module> mod_, FnRef<bool(StringView)> needsUniform_) {
	Uniformity result {};
	result.uniformStmt.assign(mod_.stmts.size(), true);
	result.requiresUniform.assign(mod_.functions.size(), false);

	for (auto f = 0uz; f < mod_.functions.size(); ++f) {
		const auto& fn = mod_.functions[f];
		if (not fn.body.has_value()) {
			continue;
		}

		FunctionWalker walker { mod_, needsUniform_, result };
		for (const auto& param : fn.params) {
			walker.mark(param.name, true);
		}
		do {
			walker.changed = false;
			walker.loopTaint = walker.fnTaint = false;
			walker.stmt(fn.body->root, false);
		} while (walker.changed);
		result.requiresUniform[f] = walker.direct;
	}

	for (bool grew = true; grew;) {
		grew = false;
		for (auto f = 0u; f < mod_.functions.size(); ++f) {
			if (result.requiresUniform[f]) {
				continue;
			}
			for (const auto callee : usageOf(mod_, f).calledFunctions) {
				if (callee < result.requiresUniform.size() && result.requiresUniform[callee]) {
					result.requiresUniform[f] = grew = true;
					break;
				}
			}
		}
	}
	return result;
}
