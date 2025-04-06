#include "Action/Action.hpp"

/**/

#include "ActionManager.hpp"

#include <Engine.Asserts/Todo.hpp>
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

Result<void, std::runtime_error> ActionManager::apply(mref<Arci<Action>> action_) const {

	// TODO: Check whether we need to synchronize
	_log->apply(clone(action_));
	const auto result = _dispatcher->apply(clone(action_));

	if (result.has_value()) {
		_log->succeed(std::move(action_));
		return result;
	}

	/**/

	const auto revokeResult = _dispatcher->revoke(clone(action_));
	if (not revokeResult.has_value()) {
		IM_CORE_ERROR("Failed to recover from invalid state after failed action.");
		::hg::panic();
	}

	_log->fail(std::move(action_));
	return result;
}

Result<void, std::runtime_error> ActionManager::undo() const {

	auto action = _log->undo();
	if (action == nullptr) {
		IM_CORE_WARN("Tried to revert on empty action log.");
		return Unexpected { std::runtime_error { "Tried to revert on empty action log." } };
	}

	const auto result = _dispatcher->undo(clone(action));

	if (result.has_value()) {
		_log->succeed(std::move(action));
		return result;
	}

	/**/

	// TODO: Check whether it is appropriate to revoke the action on undo!?
	::hg::todo_panic();
	//_log->fail(std::move(action));
}

Result<void, std::runtime_error> ActionManager::reapply() const {

	auto action = _log->reapply();
	if (action != nullptr) {
		IM_CORE_WARN("Tried to reapply on empty reverted action log.");
		return Unexpected { std::runtime_error { "Tried to reapply on empty reverted action log." } };
	}

	const auto result = _dispatcher->apply(clone(action));

	if (result.has_value()) {
		_log->succeed(std::move(action));
		return result;
	}

	/**/

	const auto revokeResult = _dispatcher->revoke(clone(action));
	if (not revokeResult.has_value()) {
		IM_CORE_ERROR("Failed to recover from invalid state after failed reapply of action.");
		::hg::panic();
	}

	_log->fail(std::move(action));
	return result;
}
