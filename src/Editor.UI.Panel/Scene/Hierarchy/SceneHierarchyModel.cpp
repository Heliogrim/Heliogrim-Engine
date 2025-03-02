#include "SceneHierarchyModel.hpp"

#include <Editor.UI.Service/Scene/Hierarchy/SceneHierarchyEntry.hpp>
#include <Editor.UI.Service/Scene/Hierarchy/SceneHierarchyService.hpp>

#include "SceneHierarchyView.hpp"

using namespace hg::editor::ui;
using namespace hg;

SceneHierarchyModel::SceneHierarchyModel(
	ref<SceneHierarchyView> view_,
	SharedPtr<service::SceneHierarchyService> service_,
	Vector<service::SceneHierarchyEntry> baseItemData_
) :
	_view(view_),
	service(::hg::move(service_)),
	baseItemData(::hg::move(baseItemData_)) {}

SceneHierarchyModel::~SceneHierarchyModel() = default;

void SceneHierarchyModel::changeSelection(ref<const selection_type> selection_) {}

void SceneHierarchyModel::regenerateTreeItems(ref<const scene_type> scene_) {

	baseItemData.clear();
	if (scene_.is<nmpt<engine::core::Universe>>()) {

		const auto& universe = *scene_.as<nmpt<engine::core::Universe>>();
		const auto levels = universe.getLevels();

		for (const auto& level : levels) {
			const auto levelActors = level->getActors();
			for (const auto& actor : levelActors) {
				baseItemData.emplace_back(actor);
			}
		}

	}

	if (scene_.is<nmpt<engine::core::Level>>()) {

		const auto& level = *scene_.as<nmpt<engine::core::Level>>();
		const auto actors = level.getActors();

		for (const auto& actor : actors) {
			baseItemData.emplace_back(actor);
		}

	}

	if (scene_.is<nmpt<Actor>>()) {
		baseItemData.emplace_back(scene_.as<nmpt<Actor>>());
	}

	/**/

	_view.displayTreeItems(
		baseItemData,
		[service = service](ref<const service::SceneHierarchyEntry> parent_, ref<Vector<service::SceneHierarchyEntry>> children_) {
			return service->fetchLayer(parent_, children_);
		}
	);
}

UniquePtr<SceneHierarchyModel> editor::ui::makeSceneHierarchyModel(
	ref<SceneHierarchyView> view_,
	SharedPtr<service::SceneHierarchyService> service_
) {
	return make_uptr<SceneHierarchyModel>(view_, ::hg::move(service_), Vector<service::SceneHierarchyEntry> {});
}
