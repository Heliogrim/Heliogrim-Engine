#include "../Action/Action.hpp"

/**/

#include "ActionDispatcher.hpp"

#include <Heliogrim/Async/Await.hpp>
#include <Heliogrim/Async/Execute.hpp>

using namespace hg::editor;
using namespace hg;

ActionDispatcher::ActionDispatcher() = default;

Result<void, std::runtime_error> ActionDispatcher::apply(mref<Arci<Action>> action_) const {
	// The dispatch owns the signal: re-arm before, set after the body.
	action_->finished().reset();
	auto result = Result<void, std::runtime_error> {};
	execute([action = action_, &result] { result = action->apply(); action->finished().set(); });
	await(action_->finished());
	return result;
}

Result<void, std::runtime_error> ActionDispatcher::revoke(mref<Arci<Action>> action_) const {
	action_->finished().reset();
	auto result = Result<void, std::runtime_error> {};
	execute([action = action_, &result] { result = action->revoke(); action->finished().set(); });
	await(action_->finished());
	return result;
}

Result<void, std::runtime_error> ActionDispatcher::undo(mref<Arci<Action>> action_) const {
	if (not action_->isReversible()) {
		return Unexpected { std::runtime_error("Failed to undo irreversible action.") };
	}

	action_->finished().reset();
	auto result = Result<void, std::runtime_error> {};
	execute([action = action_, &result] { result = action->undo(); action->finished().set(); });
	await(action_->finished());
	return result;
}
