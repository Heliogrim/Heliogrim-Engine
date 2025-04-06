#include "../Action/Action.hpp"

/**/

#include "ActionDispatcher.hpp"

#include <Heliogrim/Async/Await.hpp>
#include <Heliogrim/Async/Execute.hpp>

using namespace hg::editor;
using namespace hg;

ActionDispatcher::ActionDispatcher() = default;

Result<void, std::runtime_error> ActionDispatcher::apply(mref<Arci<Action>> action_) const {
	auto result = Result<void, std::runtime_error> {};
	execute([action = action_, &result] { result = action->apply(); });
	await(*action_);
	return result;
}

Result<void, std::runtime_error> ActionDispatcher::revoke(mref<Arci<Action>> action_) const {
	auto result = Result<void, std::runtime_error> {};
	execute([action = action_, &result] { result = action->revoke(); });
	await(*action_);
	return result;
}

Result<void, std::runtime_error> ActionDispatcher::undo(mref<Arci<Action>> action_) const {
	if (not action_->isReversible()) {
		return Unexpected { std::runtime_error("Failed to undo irreversible action.") };
	}

	auto result = Result<void, std::runtime_error> {};
	execute([action = action_, &result] { result = action->undo(); });
	await(*action_);
	return result;
}
