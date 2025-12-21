#include "EditorInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Core/HeliogrimEditor.hpp>
#include <Editor.UI.Main/EditorUI.hpp>
#include <Editor.UI.View/Scene/SceneEditorController.hpp>
#include <Engine.ACS/ActorModule.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Reflow.Uikit/Atom/Image.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>
#include <Heliogrim/ActorInitializer.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Component/Scene/UI/UIComponent.hpp>
#include <Heliogrim/Universe/Universe.hpp>

using namespace hg::editor::ui;
using namespace hg;

/**/

void editor::ui::initEditor(ref<EditorUI> editorUi_) {

	const auto& engine = *EditorEngine::getEngine();
	const auto editorSession = engine.getEditorSession();
	const auto editorUniverse = editorSession->getUniverseContext().getCurrentUniverse();

	/**/

	const auto window = engine::reflow::WindowManager::get()->requestWindow(
		"Heliogrim Engine"sv,
		math::ivec2 { 1280, 720 },
		{},
		"Editor-Renderer"sv,
		editorUniverse->getScene()
	);

	/**/

	auto actor = CreateActor();
	auto uic = actor->addComponent<UIComponent>();

	uic->setWindow(window);
	GetUniverse(HeliogrimEditor::getEditorSession()).addActor(std::move(actor));

}
