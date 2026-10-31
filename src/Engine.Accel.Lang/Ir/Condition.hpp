#pragma once

#include <utility>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::accel::lang::ir {
	struct Toggle {
		String name;
		Vector<String> values;
		String defaultValue;
	};

	/**
	 * Toggle assignment, sorted by toggle name
	 */
	using ToggleSet = Vector<std::pair<String, String>>;

	enum class ConditionKind : u8 {
		eTrue,
		eEquals,
		eNot,
		eAnd,
		eOr
	};

	struct Condition {
		ConditionKind kind = ConditionKind::eTrue;
		String toggle;
		String value;
		Vector<Condition> children;

		[[nodiscard]] static Condition always();

		[[nodiscard]] static Condition equals(String toggle_, String value_);

		[[nodiscard]] static Condition negate(Condition inner_);

		[[nodiscard]] static Condition all(Vector<Condition> children_);

		[[nodiscard]] static Condition any(Vector<Condition> children_);

		[[nodiscard]] static Condition none(Vector<Condition> children_);
	};

	[[nodiscard]] bool evaluate(ref<const Condition> condition_, ref<const ToggleSet> toggles_);

	/**
	 * Enumerates the toggle combinations (declaration order, values in declared order)
	 * and returns the first one matched by no arm.
	 */
	[[nodiscard]] Opt<ToggleSet> findUncovered(ref<const Vector<Toggle>> toggles_, ref<const Vector<Condition>> arms_);
}
