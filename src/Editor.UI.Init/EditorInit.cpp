#include "EditorInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Core/HeliogrimEditor.hpp>
#include <Editor.UI.Main/EditorUI.hpp>
#include <Editor.UI.Theme/EditorTheme.hpp>
#include <Editor.UI.View/Scene/SceneEditorController.hpp>
#include <Editor.UI.View/Scene/SceneEditorView.hpp>
#include <Editor.UI/Widget/InputBindingProvider.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp>
#include <Engine.Reflow/Window/Window.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Component/Scene/UI/UIComponent.hpp>
#include <Heliogrim/Universe/Universe.hpp>

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

/**/

static void initDefaultUi(ref<editor::EditorUI> editorUi_, nmpt<Window> window_);

/**/

void editor::ui::initEditor(ref<EditorUI> editorUi_) {

	const auto& engine = *EditorEngine::getEngine();
	const auto editorSession = engine.getEditorSession();
	const auto editorUniverse = editorSession->getUniverseContext().getCurrentUniverse();

	/**/

	const auto window = WindowManager::get()->requestWindow(
		"Heliogrim Engine"sv,
		math::ivec2 { 1280, 720 },
		{},
		"Editor-Renderer"sv,
		editorUniverse->getScene()
	);

	initDefaultUi(editorUi_, window.get());

	/**/

	auto actor = CreateActor();
	auto uic = actor->addComponent<UIComponent>();

	uic->setWindow(window);
	GetUniverse(HeliogrimEditor::getEditorSession()).addActor(std::move(actor));

	/**/

	editorUi_.getEditorModel().scene.instance.next(GetUniverse().unwrap().get());
}

/**/

static void initDefaultUi(ref<editor::EditorUI> editorUi_, nmpt<Window> window_) {

	window_->setProviderThemes({ getEditorTheming().getBaseTheme().value() });

	/**/

	auto layout = make_sptr<uikit::VerticalLayout>();

	auto& attributes = layout->getLayoutAttributes().attributeSets;
	std::get<0>(attributes).update<attr::BoxLayout::widthGrow>(1.F);
	std::get<0>(attributes).update<attr::BoxLayout::heightGrow>(1.F);
	std::get<1>(attributes).update<attr::FlexLayout::rowGap>(8.F);

	/**/

	editorUi_._sceneEditorController = make_uptr<SceneEditorController>(
		editorUi_.getEditorModel(),
		editorUi_.getEditorServices()
	);
	editorUi_._sceneEditorController->setup();

	layout->addChild(editorUi_._sceneEditorController->_view->main);

	/**/

	// TODO: Make Provider-Stack to footprint of layout/render ineffective provider widgets in sub-tree
	auto globalShortcutProvider = make_sptr<InputBindingProvider>();
	globalShortcutProvider->setContent(::hg::move(layout));
	window_->setContent(::hg::move(globalShortcutProvider));
}
