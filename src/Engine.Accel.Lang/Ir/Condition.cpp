#include "Condition.hpp"

#include <algorithm>

using namespace hg::engine::accel::lang;
using namespace hg;

ir::Condition ir::Condition::always() {
	return Condition {};
}

ir::Condition ir::Condition::equals(String toggle_, String value_) {
	return Condition { .kind = ConditionKind::eEquals, .toggle = std::move(toggle_), .value = std::move(value_) };
}

ir::Condition ir::Condition::negate(Condition inner_) {
	Condition result { .kind = ConditionKind::eNot };
	result.children.emplace_back(std::move(inner_));
	return result;
}

ir::Condition ir::Condition::all(Vector<Condition> children_) {
	return Condition { .kind = ConditionKind::eAnd, .children = std::move(children_) };
}

ir::Condition ir::Condition::any(Vector<Condition> children_) {
	return Condition { .kind = ConditionKind::eOr, .children = std::move(children_) };
}

ir::Condition ir::Condition::none(Vector<Condition> children_) {
	return negate(any(std::move(children_)));
}

bool ir::evaluate(ref<const Condition> condition_, ref<const ToggleSet> toggles_) {
	switch (condition_.kind) {
		case ConditionKind::eTrue: {
			return true;
		}
		case ConditionKind::eEquals: {
			return std::ranges::any_of(
				toggles_,
				[&](const auto& entry_) { return entry_.first == condition_.toggle && entry_.second == condition_.value; }
			);
		}
		case ConditionKind::eNot: {
			return condition_.children.empty() || not evaluate(condition_.children.front(), toggles_);
		}
		case ConditionKind::eAnd: {
			return std::ranges::all_of(
				condition_.children,
				[&](const auto& child_) { return evaluate(child_, toggles_); }
			);
		}
		case ConditionKind::eOr: {
			return std::ranges::any_of(
				condition_.children,
				[&](const auto& child_) { return evaluate(child_, toggles_); }
			);
		}
	}
	return false;
}

Opt<ir::ToggleSet> ir::findUncovered(ref<const Vector<Toggle>> toggles_, ref<const Vector<Condition>> arms_) {

	// Odometer over the cartesian product, first toggle varies slowest (declaration order)
	Vector<size_t> pos(toggles_.size(), 0uLL);
	if (std::ranges::any_of(toggles_, [](const auto& t_) { return t_.values.empty(); })) {
		return None;
	}

	while (true) {
		ToggleSet set;
		set.reserve(toggles_.size());
		for (size_t i = 0; i < toggles_.size(); ++i) {
			set.emplace_back(toggles_[i].name, toggles_[i].values[pos[i]]);
		}
		std::ranges::sort(set, [](const auto& l_, const auto& r_) { return l_.first < r_.first; });

		if (not std::ranges::any_of(arms_, [&](const auto& arm_) { return evaluate(arm_, set); })) {
			return set;
		}

		size_t at = toggles_.size();
		while (at > 0) {
			--at;
			if (++pos[at] < toggles_[at].values.size()) {
				break;
			}
			pos[at] = 0uLL;
			if (at == 0) {
				return None;
			}
		}
		if (toggles_.empty()) {
			return None;
		}
	}
}
