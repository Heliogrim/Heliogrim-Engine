#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::ui {
	struct EditorUiModel;
	struct EditorUiServices;

	struct SceneEditorView;

	class AssetBrowserController;
	class PropertyController;
	class SceneHierarchyController;
}

namespace hg::editor::ui {
	class SceneEditorController {
	public:
		SceneEditorController(ref<EditorUiModel> editorModel_, ref<const EditorUiServices> editorServices_);

		~SceneEditorController();

	private:
		ref<EditorUiModel> _editorModel;
		ref<const EditorUiServices> _editorServices;

	private:
	public:
		UniquePtr<SceneEditorView> _view;

	public:
		UniquePtr<AssetBrowserController> assetBrowser;
		UniquePtr<PropertyController> properties;
		UniquePtr<SceneHierarchyController> sceneHierarchy;

	public:
		void setup();
	};
}
