#include "ActionManager.hpp"

#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Action/Action.hpp"
#include "Log/ActionDispatcher.hpp"
#include "Log/ActionLog.hpp"

using namespace hg::editor;
using namespace hg;

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

cref<ActionManager> ActionManager::apply(mref<Arci<Action>> action_) const {

	// TODO: Check whether we need to synchronize
	_log->apply(clone(action_));
	const auto result = (*_dispatcher)(clone(action_));

	if (result) {
		_log->succeed(std::move(action_));
	} else {
		_log->fail(std::move(action_));
	}
	return *this;
}

void ActionManager::revert() const {

	auto action = _log->revert();
	if (action == nullptr) {
		IM_CORE_WARN("Tried to revert on empty action log.");
		return;
	}

	const auto result = (*_dispatcher)(clone(action));

	if (result) {
		_log->succeed(std::move(action));
	} else {
		_log->fail(std::move(action));
	}
}

void ActionManager::reapply() const {

	auto action = _log->reapply();
	if (action != nullptr) {
		IM_CORE_WARN("Tried to reapply on empty reverted action log.");
		return;
	}

	const auto result = (*_dispatcher)(clone(action));

	if (result) {
		_log->succeed(std::move(action));
	} else {
		_log->fail(std::move(action));
	}
}
