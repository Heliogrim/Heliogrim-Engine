#include "GenerateProjectStructure.hpp"

#include <Engine.Asserts/Panic.hpp>
#include <Engine.Asserts/Todo.hpp>

using namespace hg::editor;
using namespace hg;

GenerateProjectStructureSubAction::GenerateProjectStructureSubAction(cref<Action> parentAction_) :
	InheritMeta(parentAction_) {}

bool GenerateProjectStructureSubAction::isReversible() const noexcept {
	return false;
}

bool GenerateProjectStructureSubAction::isPending() const noexcept {
	::hg::todo_panic();
}

bool GenerateProjectStructureSubAction::isRunning() const noexcept {
	::hg::todo_panic();
}

bool GenerateProjectStructureSubAction::isFinished() const noexcept {
	::hg::todo_panic();
}

void GenerateProjectStructureSubAction::apply() {
	::hg::todo_panic();
}

void GenerateProjectStructureSubAction::reverse() {
	::hg::panic();
}

GenerateProjectStructureSubAction::operator ptr<await_signal_sub_type>() const noexcept {
	::hg::todo_panic();
}

bool GenerateProjectStructureSubAction::failed() const noexcept {
	::hg::todo_panic();
}
