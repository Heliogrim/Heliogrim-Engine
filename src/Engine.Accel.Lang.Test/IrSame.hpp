#pragma once

#include <algorithm>

#include <Engine.Accel.Lang/Ir/Module.hpp>

/* Structural equality of whole modules (Module has no operator==); refs compare by index */
namespace AccelLang {
	using namespace hg::engine::accel::lang::ir;
	using namespace hg;

	[[nodiscard]] inline bool same(ref<const Condition> a_, ref<const Condition> b_) {
		if (a_.kind != b_.kind || a_.toggle != b_.toggle || a_.value != b_.value || a_.children.size() != b_.children.size()) {
			return false;
		}
		for (auto i = 0uz; i < a_.children.size(); ++i) {
			if (not same(a_.children[i], b_.children[i])) {
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] inline bool same(ref<const FieldDesc> a_, ref<const FieldDesc> b_) {
		return a_.name == b_.name && a_.type == b_.type;
	}

	[[nodiscard]] inline bool same(ref<const TypeDesc> a_, ref<const TypeDesc> b_) {
		return a_.kind == b_.kind && a_.scalar == b_.scalar && a_.bits == b_.bits && a_.n == b_.n && a_.m == b_.m &&
			a_.element == b_.element && a_.count == b_.count && a_.name == b_.name && a_.dim == b_.dim &&
			std::ranges::equal(a_.fields, b_.fields, [](const auto& x_, const auto& y_) { return same(x_, y_); });
	}

	[[nodiscard]] inline bool same(ref<const Toggle> a_, ref<const Toggle> b_) {
		return a_.name == b_.name && a_.values == b_.values && a_.defaultValue == b_.defaultValue;
	}

	[[nodiscard]] inline bool same(ref<const BindingDecl> a_, ref<const BindingDecl> b_) {
		return a_.id == b_.id && a_.type == b_.type && a_.mode == b_.mode && a_.bindClass == b_.bindClass &&
			a_.implicit == b_.implicit && a_.input == b_.input && same(a_.when, b_.when) && a_.localName == b_.localName;
	}

	[[nodiscard]] inline bool same(ref<const FunctionDecl> a_, ref<const FunctionDecl> b_) {
		const auto params = std::ranges::equal(
			a_.params,
			b_.params,
			[](const auto& x_, const auto& y_) { return x_.name == y_.name && x_.type == y_.type; }
		);
		const auto body = a_.body.has_value() == b_.body.has_value() &&
			(not a_.body.has_value() || a_.body->root.index == b_.body->root.index);
		return a_.name == b_.name && params && a_.ret == b_.ret && body && a_.redefineAllowed == b_.redefineAllowed &&
			same(a_.when, b_.when);
	}

	[[nodiscard]] inline bool same(ref<const Expr> a_, ref<const Expr> b_) {
		return a_.kind == b_.kind && a_.name == b_.name && a_.binding.index == b_.binding.index &&
			a_.function.index == b_.function.index && a_.local == b_.local && a_.type == b_.type &&
			std::ranges::equal(a_.operands, b_.operands, [](auto x_, auto y_) { return x_.index == y_.index; });
	}

	[[nodiscard]] inline bool same(ref<const Stmt> a_, ref<const Stmt> b_) {
		return a_.kind == b_.kind && a_.name == b_.name && a_.type == b_.type && a_.lhs.index == b_.lhs.index &&
			a_.rhs.index == b_.rhs.index &&
			std::ranges::equal(a_.children, b_.children, [](auto x_, auto y_) { return x_.index == y_.index; }) &&
			same(a_.when, b_.when);
	}

	[[nodiscard]] inline bool same(ref<const Module> a_, ref<const Module> b_) {
		const auto each = [](const auto& x_, const auto& y_) {
			return std::ranges::equal(x_, y_, [](const auto& l_, const auto& r_) { return same(l_, r_); });
		};
		return a_.name == b_.name && a_.entry == b_.entry && each(a_.toggles, b_.toggles) && each(a_.types, b_.types) &&
			each(a_.bindings, b_.bindings) && each(a_.functions, b_.functions) && each(a_.exprs, b_.exprs) &&
			each(a_.stmts, b_.stmts);
	}
}
