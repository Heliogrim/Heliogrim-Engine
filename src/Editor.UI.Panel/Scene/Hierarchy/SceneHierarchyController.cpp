#include "SceneHierarchyController.hpp"

#include <Editor.UI.Service/Scene/Hierarchy/SceneHierarchyEntry.hpp>

#include "SceneHierarchyModel.hpp"
#include "SceneHierarchyView.hpp"

using namespace hg::editor::ui;
using namespace hg;

SceneHierarchyController::SceneHierarchyController(
	ref<EditorUiModel> editor_
) :
	_editor(editor_),
	_model(),
	_view(),
	_selectSubscribe(
		editor_.scene.selected.subscribe(
			[&](const auto& selection_) {
				if (_model != nullptr) {
					_model->changeSelection(selection_);
				}
			}
		)
	),
	_sceneSubscribe(
		editor_.scene.instance.subscribe(
			[&](const auto& scene_) {
				if (_model != nullptr) {
					_model->regenerateTreeItems(scene_);
				}
			}
		)
	) {}

SceneHierarchyController::~SceneHierarchyController() {
	::hg::move(_sceneSubscribe).unsubscribe();
	::hg::move(_selectSubscribe).unsubscribe();
}

void SceneHierarchyController::setup(mref<SceneHierarchyOptions> options_) {
	_view = makeSceneHierarchyView(*this);
	_model = makeSceneHierarchyModel(*_view, ::hg::move(options_.service));
}

ref<const SceneHierarchyModel> SceneHierarchyController::getModel() const noexcept {
	return *_model;
}

ref<const SceneHierarchyView> SceneHierarchyController::getView() const noexcept {
	return *_view;
}

void SceneHierarchyController::onSelectionChange(std::span<service::SceneHierarchyEntry> selected_) {

	if (selected_.empty()) {
		_editor.scene.selected.next(nullptr);
		return;
	}

	const auto& first = selected_.front();
	if (first.value.is<nmpt<Actor>>()) {
		_editor.scene.selected.next(first.value.as<nmpt<Actor>>());
		return;
	}

	if (first.value.is<nmpt<HierarchyComponent>>()) {
		_editor.scene.selected.next(first.value.as<nmpt<HierarchyComponent>>());
		return;
	}

}

UniquePtr<SceneHierarchyController> editor::ui::makeSceneHierarchy(SceneHierarchyOptions options_) {
	auto controller = make_uptr<SceneHierarchyController>(options_.editorUiModel);
	controller->setup(::hg::move(options_));
	return controller;
}
