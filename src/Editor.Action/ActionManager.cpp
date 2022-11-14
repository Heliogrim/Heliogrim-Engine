#include "ActionManager.hpp"

#include <Engine.Logging/Logger.hpp>

#include "Log/ActionDispatcher.hpp"
#include "Log/ActionLog.hpp"

using namespace ember::editor;
using namespace ember;

ptr<ActionManager> ActionManager::_instance = nullptr;

const ptr<ActionManager> ActionManager::get() {
    return _instance;
}

const ptr<ActionManager::this_type> ActionManager::make() {

    if (_instance == nullptr) {
        _instance = new ActionManager();
    }

    return _instance;
}

void ActionManager::destroy() {
    delete _instance;
    _instance = nullptr;
}

ActionManager::ActionManager() :
    _log(new ActionLog),
    _dispatcher(new ActionDispatcher) {}

ActionManager::~ActionManager() {
    tidy();
}

void ActionManager::tidy() {

    // TODO: if (_log && _log->hasPendingState())

    if (_dispatcher) {
        delete _dispatcher;
        _dispatcher = nullptr;
    }

    if (_log) {
        delete _log;
        _log = nullptr;
    }
}

void ActionManager::apply(cref<sptr<Action>> action_) const {

    // TODO: Check whether we need to synchronize
    _log->apply(action_);
    const auto result = (*_dispatcher)(action_);

    if (result) {
        _log->succeed(action_);
    } else {
        _log->fail(action_);
    }
}

void ActionManager::revert() const {

    const auto action = _log->revert();
    if (not action) {
        IM_CORE_WARN("Tried to revert on empty action log.");
        return;
    }

    const auto result = (*_dispatcher)(action);

    if (result) {
        _log->succeed(action);
    } else {
        _log->fail(action);
    }
}

void ActionManager::reapply() const {

    const auto action = _log->reapply();
    if (not action) {
        IM_CORE_WARN("Tried to reapply on empty reverted action log.");
        return;
    }

    const auto result = (*_dispatcher)(action);

    if (result) {
        _log->succeed(action);
    } else {
        _log->fail(action);
    }
}
