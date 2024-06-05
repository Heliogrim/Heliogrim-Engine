#include "ProjectInit.hpp"

#include <Editor.Action/ActionManager.hpp>
#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Project.Generate/Action/NewProject.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Resource/File.hpp>

using namespace hg::editor::boot;
using namespace hg;

void editor::boot::initProject() {

	const auto engine = EditorEngine::getEngine();
	const auto& actions = *ActionManager::make();

	/**/

	const auto testPath = fs::Url { "file"sv, fs::Path { "X:/Heliogrim-Projects/ProjectInit"sv } };
	if (not std::filesystem::exists(testPath.path().stdFsPath())) {
		actions.apply(Arci<NewProjectAction>::create("Init Project", clone(testPath)));
	}
}
