#pragma once

#include <Editor.UI.Main/EditorUiModel.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Filesystem/Url.hpp>

namespace hg::editor::ui {
	class SceneHierarchyModel;
	class SceneHierarchyView;
}

namespace hg::editor::ui::service {
	class SceneHierarchyService;
	struct SceneHierarchyEntry;
}

namespace hg::editor::ui {
	struct SceneHierarchyOptions {
		ref<EditorUiModel> editorUiModel;
		SharedPtr<service::SceneHierarchyService> service;
	};

	/**/

	class SceneHierarchyController {
	public:
		SceneHierarchyController(ref<EditorUiModel> editor_);

		~SceneHierarchyController();

	private:
		ref<EditorUiModel> _editor;
		UniquePtr<SceneHierarchyModel> _model;
		UniquePtr<SceneHierarchyView> _view;

		decltype(EditorUiModel::Scene::selected)::subscription_type _selectSubscribe;
		decltype(EditorUiModel::Scene::instance)::subscription_type _sceneSubscribe;

	public:
		void setup(mref<SceneHierarchyOptions> options_);

	public:
		[[nodiscard]] ref<const SceneHierarchyModel> getModel() const noexcept;

		[[nodiscard]] ref<const SceneHierarchyView> getView() const noexcept;

	public:
		void onSelectionChange(std::span<service::SceneHierarchyEntry> selected_);
	};

	/**/

	[[nodiscard]] extern UniquePtr<SceneHierarchyController> makeSceneHierarchy(SceneHierarchyOptions options_);
}
