#include "SceneEditorController.hpp"

#include <Editor.UI.Main/EditorUiServices.hpp>

#include "SceneEditorView.hpp"

import Editor.UI.Panel;

using namespace hg::editor::ui;
using namespace hg;

SceneEditorController::SceneEditorController(ref<EditorUiModel> editorModel_, ref<const EditorUiServices> editorServices_) :
	_editorModel(editorModel_),
	_editorServices(editorServices_) {}

SceneEditorController::~SceneEditorController() {}

void SceneEditorController::setup() {

	assetBrowser = makeAssetBrowser({ .service = _editorServices.assetBrowserService, .basePath = fs::Path {}, .initialPath = None });
	properties = makeProperty({ .editorUiModel = _editorModel, .service = _editorServices.propertyService });
	sceneHierarchy = makeSceneHierarchy({ .editorUiModel = _editorModel, .service = _editorServices.sceneHierarchyService });

	/**/

	_view = makeSceneEditorView(*this);
}
