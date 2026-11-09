#include "../Action/Action.hpp"

/**/

#include "ActionDispatcher.hpp"

#include <Heliogrim/Async/Await.hpp>
#include <Heliogrim/Async/Execute.hpp>

using namespace hg::editor;
using namespace hg;

namespace {
	// A throwing action must not strand the dispatcher.
	struct SetOnExit {
		ref<Signal> signal;

		~SetOnExit() {
			signal.set();
		}
	};
}

ActionDispatcher::ActionDispatcher() = default;

Result<void, std::runtime_error> ActionDispatcher::apply(mref<Arci<Action>> action_) const {
	// The dispatch owns the signal: re-arm before, set after the body.
	action_->finished().reset();
	auto result = Result<void, std::runtime_error> {};
	execute(
		[action = action_, &result] {
			SetOnExit guard { action->finished() };
			result = action->apply();
		}
	);
	await(action_->finished());
	return result;
}

Result<void, std::runtime_error> ActionDispatcher::revoke(mref<Arci<Action>> action_) const {
	action_->finished().reset();
	auto result = Result<void, std::runtime_error> {};
	execute(
		[action = action_, &result] {
			SetOnExit guard { action->finished() };
			result = action->revoke();
		}
	);
	await(action_->finished());
	return result;
}

Result<void, std::runtime_error> ActionDispatcher::undo(mref<Arci<Action>> action_) const {
	if (not action_->isReversible()) {
		return Unexpected { std::runtime_error("Failed to undo irreversible action.") };
	}

	action_->finished().reset();
	auto result = Result<void, std::runtime_error> {};
	execute(
		[action = action_, &result] {
			SetOnExit guard { action->finished() };
			result = action->undo();
		}
	);
	await(action_->finished());
	return result;
}
