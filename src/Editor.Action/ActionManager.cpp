#include "ActionManager.hpp"

#include <Engine.Logging/Logger.hpp>

#include "Engine.Common/Make.hpp"
#include "Log/ActionDispatcher.hpp"
#include "Log/ActionLog.hpp"

using namespace hg::editor;
using namespace hg;

ActionManager::ActionManager(ref<engine::Engine> engine_) :
	SubModule(std::addressof(engine_)),
	_deps(),
	_dispatcher(),
	_log() {}

ActionManager::~ActionManager() {
	ActionManager::destroy();
}

engine::core::DependencyKey ActionManager::moduleKey() const noexcept {
	return ActionDepKey;
}

cref<CompactSet<engine::core::SubModuleDependency>> ActionManager::dependencies() const noexcept {
	return _deps;
}

void ActionManager::setup() {
	_dispatcher = make_uptr<ActionDispatcher>();
	_log = make_uptr<ActionLog>();
}

void ActionManager::start() {}

void ActionManager::stop() {}

void ActionManager::destroy() {
	// TODO: if (_log && _log->hasPendingState())

	_dispatcher.reset();
	_log.reset();
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
