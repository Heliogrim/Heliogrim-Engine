#include "LoadProject.hpp"

#include <Engine.Asserts/Panic.hpp>
#include <Engine.Asserts/Todo.hpp>

using namespace hg::editor;
using namespace hg;

LoadProjectSubAction::LoadProjectSubAction(cref<Action> parentAction_) :
	InheritMeta(parentAction_) {}

bool LoadProjectSubAction::isReversible() const noexcept {
	return false;
}

bool LoadProjectSubAction::isPending() const noexcept {
	::hg::todo_panic();
}

bool LoadProjectSubAction::isRunning() const noexcept {
	::hg::todo_panic();
}

bool LoadProjectSubAction::isFinished() const noexcept {
	::hg::todo_panic();
}

void LoadProjectSubAction::apply() {
	::hg::todo_panic();
}

void LoadProjectSubAction::reverse() {
	::hg::panic();
}

LoadProjectSubAction::operator ptr<await_signal_sub_type>() const noexcept {
	::hg::todo_panic();
}

bool LoadProjectSubAction::failed() const noexcept {
	::hg::todo_panic();
}
