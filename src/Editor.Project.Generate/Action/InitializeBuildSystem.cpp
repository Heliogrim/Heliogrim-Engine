#include "InitializeBuildSystem.hpp"

#include <Engine.Asserts/Panic.hpp>
#include <Engine.Asserts/Todo.hpp>

using namespace hg::editor;
using namespace hg;

InitializeBuildSystemSubAction::InitializeBuildSystemSubAction(cref<Action> parentAction_) :
	InheritMeta(parentAction_) {}

bool InitializeBuildSystemSubAction::isReversible() const noexcept {
	return false;
}

bool InitializeBuildSystemSubAction::isPending() const noexcept {
	::hg::todo_panic();
}

bool InitializeBuildSystemSubAction::isRunning() const noexcept {
	::hg::todo_panic();
}

bool InitializeBuildSystemSubAction::isFinished() const noexcept {
	::hg::todo_panic();
}

void InitializeBuildSystemSubAction::apply() {
	::hg::todo_panic();
}

void InitializeBuildSystemSubAction::reverse() {
	::hg::panic();
}

InitializeBuildSystemSubAction::operator ptr<await_signal_sub_type>() const noexcept {
	::hg::todo_panic();
}

bool InitializeBuildSystemSubAction::failed() const noexcept {
	::hg::todo_panic();
}
