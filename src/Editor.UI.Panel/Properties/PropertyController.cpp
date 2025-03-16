#include "PropertyController.hpp"

#include "PropertyModel.hpp"
#include "PropertyView.hpp"

using namespace hg::editor::ui;
using namespace hg;

PropertyController::PropertyController(ref<EditorUiModel> editor_) :
	_editor(editor_),
	_model(),
	_view(),
	_selectSubscribe(
		editor_.scene.selected.subscribe(
			[&](const auto& selection_) {
				if (_model != nullptr) {
					_model->changeSubject(selection_);
				}
			}
		)
	) {}

PropertyController::~PropertyController() {
	::hg::move(_selectSubscribe).unsubscribe();
}

void PropertyController::setup(mref<PropertyOptions> options_) {
	_view = makePropertyView(*this);
	_model = makePropertyModel(*_view, ::hg::move(options_.service));
}

ref<const PropertyModel> PropertyController::getModel() const noexcept {
	return *_model;
}

ref<const PropertyView> PropertyController::getView() const noexcept {
	return *_view;
}

UniquePtr<PropertyController> editor::ui::makeProperty(mref<PropertyOptions> options_) {
	auto controller = make_uptr<PropertyController>(options_.editorUiModel);
	controller->setup(::hg::move(options_));
	return controller;
}
