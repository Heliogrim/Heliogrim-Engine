#include "LinkEditorResources.hpp"

#include <Engine.Asserts/Panic.hpp>
#include <Engine.Asserts/Todo.hpp>

using namespace hg::editor;
using namespace hg;

LinkEditorResourcesSubAction::LinkEditorResourcesSubAction(cref<Action> parentAction_) :
	InheritMeta(parentAction_) {}

bool LinkEditorResourcesSubAction::isReversible() const noexcept {
	return false;
}

bool LinkEditorResourcesSubAction::isPending() const noexcept {
	::hg::todo_panic();
}

bool LinkEditorResourcesSubAction::isRunning() const noexcept {
	::hg::todo_panic();
}

bool LinkEditorResourcesSubAction::isFinished() const noexcept {
	::hg::todo_panic();
}

void LinkEditorResourcesSubAction::apply() {
	::hg::todo_panic();
}

void LinkEditorResourcesSubAction::reverse() {
	::hg::panic();
}

LinkEditorResourcesSubAction::operator ptr<await_signal_sub_type>() const noexcept {
	::hg::todo_panic();
}

bool LinkEditorResourcesSubAction::failed() const noexcept {
	::hg::todo_panic();
}
