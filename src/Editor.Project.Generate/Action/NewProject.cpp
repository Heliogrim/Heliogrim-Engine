#include "NewProject.hpp"

#include <Editor.Action/ActionManager.hpp>
#include <Engine.Asserts/Panic.hpp>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Managed/Rc.hpp>

#include "GenerateProjectStructure.hpp"
#include "InitializeBuildSystem.hpp"
#include "LinkEditorResources.hpp"
#include "LoadProject.hpp"

using namespace hg::editor;
using namespace hg;

NewProjectAction::NewProjectAction(mref<String> projectName_, mref<fs::Url> projectBasePath_) :
	InheritMeta(),
	_projectName(std::move(projectName_)),
	_projectBasePath(std::move(projectBasePath_)) {}

NewProjectAction::~NewProjectAction() = default;

String NewProjectAction::getProjectName() const noexcept {
	return _projectName;
}

fs::Url NewProjectAction::getProjectBasePath() const noexcept {
	return _projectBasePath;
}

bool NewProjectAction::isReversible() const noexcept {
	return false;
}

bool NewProjectAction::isPending() const noexcept {
	::hg::todo_panic();
}

bool NewProjectAction::isRunning() const noexcept {
	::hg::todo_panic();
}

bool NewProjectAction::isFinished() const noexcept {
	::hg::todo_panic();
}

void NewProjectAction::apply() {
	// Warning: Hacky workaround, cause we don't have sub-actions at the moment
	// TODO: Introduce editor sub-actions or compound actions
	::hg::execute(
		[parent = arci_from_this()]() {
			(*ActionManager::get())
				.apply(Arci<GenerateProjectStructureSubAction>::create(*parent))
				.apply(Arci<LinkEditorResourcesSubAction>::create(*parent))
				.apply(Arci<InitializeBuildSystemSubAction>::create(*parent))
				.apply(Arci<LoadProjectSubAction>::create(*parent));
		}
	);
}

void NewProjectAction::reverse() {
	::hg::panic();
}

NewProjectAction::operator ptr<await_signal_sub_type>() const noexcept {
	::hg::todo_panic();
}

bool NewProjectAction::failed() const noexcept {
	::hg::todo_panic();
}
