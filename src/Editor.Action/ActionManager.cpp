#include "Action/Action.hpp"

/**/

#include "ActionManager.hpp"

#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Log/ActionDispatcher.hpp"
#include "Log/ActionLog.hpp"

using namespace hg::editor;
using namespace hg;

ActionManager::ActionManager(ref<engine::Engine> engine_) :
	SubModule(std::addressof(engine_)),
	_deps(),
	_log(),
	_dispatcher() {}

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
