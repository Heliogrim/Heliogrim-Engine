#include "ActionLog.hpp"

#include "ActionSaveState.hpp"

#include <Ember/Inbuilt.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace ember::editor;
using namespace ember;

ActionLog::ActionLog() :
    _log(),
    _relog(),
    _saveState(NULL) {}

void ActionLog::storeLog(cref<sptr<Action>> entry_) {

    if (_log.size() >= action_log_size) {
        _log.pop_front();
    }

    _log.push_back(entry_);
}

sptr<Action> ActionLog::popLog() {

    if (_log.empty()) {
        return nullptr;
    }

    sptr<Action> peek { _log.back() };
    _log.pop_back();

    return peek;
}

void ActionLog::storeRevertLog(cref<sptr<Action>> entry_) {
    // TODO: Check how many entries we may allow to be re-applyable
    _relog.push(entry_);
}

sptr<Action> ActionLog::popRevertLog() {

    if (_relog.empty()) {
        return nullptr;
    }

    sptr<Action> peek { _relog.top() };
    _relog.pop();
    return peek;
}

void ActionLog::storeActionState(cref<sptr<Action>> action_) {

    ptr<ActionSaveState> state { make_ptr<ActionSaveState>() };

    uintptr_t expect { NULL };
    while (not _saveState.compare_exchange_strong(expect, reinterpret_cast<uintptr_t>(state), _STD memory_order::seq_cst)) {
        expect = NULL;
        ::ember::yield();
    }

    // TODO: Store state / serialize effected data to restore on fail
    state;
}

void ActionLog::dropActionState() {

    uintptr_t stored { _saveState.load() };
    if (not _saveState.compare_exchange_strong(stored, NULL, _STD memory_order::seq_cst)) {
        return;
    }

    ptr<ActionSaveState> state { reinterpret_cast<ptr<ActionSaveState>>(stored) };
    // TODO: Cleanup state / serialized effected data
    delete state;
}

bool ActionLog::revertActionState(cref<sptr<Action>> action_) {

    uintptr_t stored { _saveState.load() };
    if (not _saveState.compare_exchange_strong(stored, NULL, _STD memory_order::seq_cst)) {
        return false;
    }

    ptr<ActionSaveState> state { reinterpret_cast<ptr<ActionSaveState>>(stored) };
    // TODO: Restore state / serialized data to revert effects
    delete state;

    return true;
}

void ActionLog::apply(cref<sptr<Action>> action_) {
    storeLog(action_);
    storeActionState(action_);
}

sptr<Action> ActionLog::revert() {

    auto action = popLog();
    if (not action) {
        return action;
    }

    storeActionState(action);
    return action;
}

sptr<Action> ActionLog::reapply() {

    auto action = popRevertLog();
    if (not action) {
        return action;
    }

    storeActionState(action);
    return action;
}

void ActionLog::succeed(cref<sptr<Action>> action_) {
    dropActionState();
}

void ActionLog::fail(cref<sptr<Action>> action_) {

    const auto result = revertActionState(action_);
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
