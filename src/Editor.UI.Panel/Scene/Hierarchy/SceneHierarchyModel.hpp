#pragma once

#include <Editor.UI.Main/EditorUiModel.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

namespace hg::editor::ui {
	class SceneHierarchyView;
}

namespace hg::editor::ui::service {
	class SceneHierarchyService;
	struct SceneHierarchyEntry;
}

namespace hg::editor::ui {
	class SceneHierarchyModel {
	public:
		using scene_type = typename decltype(EditorUiModel::Scene::instance)::holder_type::type;
		using selection_type = typename decltype(EditorUiModel::Scene::selected)::holder_type::type;

	public:
		SceneHierarchyModel(
			ref<SceneHierarchyView> view_,
			SharedPtr<service::SceneHierarchyService> service_,
			Vector<service::SceneHierarchyEntry> baseItemData_
		);

		~SceneHierarchyModel();

	private:
		ref<SceneHierarchyView> _view;

	public:
		SharedPtr<service::SceneHierarchyService> service;
		Vector<service::SceneHierarchyEntry> baseItemData;

	public:
		void changeSelection(ref<const selection_type> selection_);

		void regenerateTreeItems(ref<const scene_type> scene_);
	};

	/**/

	[[nodiscard]] extern UniquePtr<SceneHierarchyModel> makeSceneHierarchyModel(
		_Inout_ ref<SceneHierarchyView> view_,
		SharedPtr<service::SceneHierarchyService> service_
	);
}
