#include "ImportProject.hpp"

#include <Engine.Asserts/Todo.hpp>

using namespace hg::editor;
using namespace hg;

ImportProjectAction::ImportProjectAction() = default;

ImportProjectAction::~ImportProjectAction() = default;

bool ImportProjectAction::isReversible() const noexcept {
	::hg::todo_panic();
}

bool ImportProjectAction::isPending() const noexcept {
	::hg::todo_panic();
}

bool ImportProjectAction::isRunning() const noexcept {
	::hg::todo_panic();
}

bool ImportProjectAction::isFinished() const noexcept {
	::hg::todo_panic();
}

void ImportProjectAction::apply() {
	::hg::todo_panic();
}

void ImportProjectAction::reverse() {
	::hg::todo_panic();
}

ImportProjectAction::operator ptr<await_signal_sub_type>() const noexcept {
	::hg::todo_panic();
}

bool ImportProjectAction::failed() const noexcept {
	::hg::todo_panic();
}
