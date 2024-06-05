#include "ActionDispatcher.hpp"

#include <Heliogrim/Inbuilt.hpp>

#include "../Action/Action.hpp"

using namespace hg::editor;
using namespace hg;

ActionDispatcher::ActionDispatcher() = default;

bool ActionDispatcher::operator()(mref<Arci<Action>> action_, const bool reverting_) const {

	if (reverting_ && not action_->isReversible()) {
		return false;
	}

	if (reverting_) {
		execute(
			[action = action_]() {
				action->reverse();
			}
		);

	} else {
		execute(
			[action = action_]() {
				action->apply();
			}
		);
	}

	await(*action_);
	return not action_->failed();
}
