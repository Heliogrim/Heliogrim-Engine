#include "../Action/Action.hpp"

/**/

#include "ActionLog.hpp"

#include <Engine.Common/Discard.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/Async/Yield.hpp>

using namespace hg::editor;
using namespace hg;

ActionLog::ActionLog() :
	_log(),
	_relog(),
	_saveState(NULL) {}

void ActionLog::storeLog(mref<Arci<Action>> entry_) {
	if (_log.size() >= action_log_size) {
		_log.pop_front();
	}

	_log.push_back(entry_);
}

Arci<Action> ActionLog::popLog() {
	if (_log.empty()) {
		return nullptr;
	}

	Arci<Action> peek { _log.back() };
	_log.pop_back();

	return peek;
}

void ActionLog::storeUndoLog(mref<Arci<Action>> entry_) {
	// TODO: Check how many entries we may allow to be re-applyable
	_relog.push(entry_);
}

Arci<Action> ActionLog::popUndoLog() {
	if (_relog.empty()) {
		return nullptr;
	}

	Arci<Action> peek { _relog.top() };
	_relog.pop();
	return peek;
}

void ActionLog::apply(mref<Arci<Action>> action_) {
	storeLog(clone(action_));
}

Arci<Action> ActionLog::undo() {
	return popLog();
}

Arci<Action> ActionLog::reapply() {
	return popUndoLog();
}

void ActionLog::succeed(mref<Arci<Action>> action_) {
	::hg::discard(::hg::move(action_));
}

void ActionLog::fail(mref<Arci<Action>> action_) {

	const auto stored = popLog();
	if (stored != action_) {
		IM_CORE_ERROR("Mismatch of logged action while reverting, due to previous failure.");
	}

	// TODO: Check whether we want to store failed actions to reverted log to reapply the same action again / retry
}
