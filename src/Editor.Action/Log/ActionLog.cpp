#include "ActionLog.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "ActionSaveState.hpp"
#include "../Action/Action.hpp"

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

void ActionLog::storeRevertLog(mref<Arci<Action>> entry_) {
	// TODO: Check how many entries we may allow to be re-applyable
	_relog.push(entry_);
}

Arci<Action> ActionLog::popRevertLog() {
	if (_relog.empty()) {
		return nullptr;
	}

	Arci<Action> peek { _relog.top() };
	_relog.pop();
	return peek;
}

void ActionLog::storeActionState(mref<Arci<Action>> action_) {
	ptr<ActionSaveState> state { make_ptr<ActionSaveState>() };

	uintptr_t expect { NULL };
	while (not _saveState.compare_exchange_strong(
		expect,
		reinterpret_cast<uintptr_t>(state),
		std::memory_order::seq_cst
	)) {
		expect = NULL;
		::hg::yield();
	}

	// TODO: Store state / serialize effected data to restore on fail
	state;
}

void ActionLog::dropActionState() {
	uintptr_t stored { _saveState.load() };
	if (not _saveState.compare_exchange_strong(stored, NULL, std::memory_order::seq_cst)) {
		return;
	}

	auto* state = reinterpret_cast<ptr<ActionSaveState>>(stored);
	// TODO: Cleanup state / serialized effected data
	delete state;
}

bool ActionLog::revertActionState(mref<Arci<Action>> action_) {
	uintptr_t stored { _saveState.load() };
	if (not _saveState.compare_exchange_strong(stored, NULL, std::memory_order::seq_cst)) {
		return false;
	}

	auto* state = reinterpret_cast<ptr<ActionSaveState>>(stored);
	// TODO: Restore state / serialized data to revert effects
	delete state;

	return true;
}

void ActionLog::apply(mref<Arci<Action>> action_) {
	storeLog(clone(action_));
	storeActionState(std::move(action_));
}

Arci<Action> ActionLog::revert() {
	auto action = popLog();
	if (action == nullptr) {
		return action;
	}

	storeActionState(clone(action));
	return action;
}

Arci<Action> ActionLog::reapply() {
	auto action = popRevertLog();
	if (action == nullptr) {
		return action;
	}

	storeActionState(clone(action));
	return action;
}

void ActionLog::succeed(mref<Arci<Action>> action_) {
	dropActionState();
}

void ActionLog::fail(mref<Arci<Action>> action_) {
	const auto result = revertActionState(std::move(action_));
	if (not result) {
		IM_CORE_ERROR("Failed to revert action state.");
	}

	#ifdef _DEBUG
    const auto stored { popLog() };
    if (stored != action_) {
        IM_CORE_ERROR("Mismatch of logged action while reverting failed one.");
    }
	#else
	popLog();
	#endif

	// TODO: Check whether we want to store failed actions to reverted log to reapply the same action again / retry
}
