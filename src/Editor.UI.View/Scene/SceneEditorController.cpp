#include "SceneEditorController.hpp"

#include <Editor.UI.Main/EditorUiServices.hpp>
#include <Editor.UI.Panel/Assets/AssetBrowserController.hpp>
#include <Editor.UI.Panel/Properties/PropertyController.hpp>
#include <Editor.UI.Panel/Scene/Hierarchy/SceneHierarchyController.hpp>

#include "SceneEditorView.hpp"

using namespace hg::editor::ui;
using namespace hg;

SceneEditorController::SceneEditorController(ref<EditorUiModel> editorModel_, ref<const EditorUiServices> editorServices_) :
	_editorModel(editorModel_),
	_editorServices(editorServices_) {}

SceneEditorController::~SceneEditorController() {}

void SceneEditorController::setup() {

	assetBrowser = makeAssetBrowser({ .service = _editorServices.assetBrowserService, .basePath = fs::Url {}, .initialPath = None });
	properties = makeProperty({ .editorUiModel = _editorModel, .service = _editorServices.propertyService });
	sceneHierarchy = makeSceneHierarchy({ .editorUiModel = _editorModel, .service = _editorServices.sceneHierarchyService });

	/**/

	_view = makeSceneEditorView(*this);
}
