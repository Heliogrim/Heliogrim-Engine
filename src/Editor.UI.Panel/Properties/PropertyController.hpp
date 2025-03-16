#pragma once

#include <Editor.UI.Main/EditorUiModel.hpp>

namespace hg::editor::ui {
	class PropertyModel;
	class PropertyView;
}

namespace hg::editor::ui::service {
	class PropertyService;
}

namespace hg::editor::ui {
	struct PropertyOptions {
		ref<EditorUiModel> editorUiModel;
		SharedPtr<service::PropertyService> service;
	};

	class PropertyController {
	public:
		PropertyController(ref<EditorUiModel> editor_);

		~PropertyController();

	private:
		ref<EditorUiModel> _editor;
		UniquePtr<PropertyModel> _model;
		UniquePtr<PropertyView> _view;

		decltype(EditorUiModel::Scene::selected)::subscription_type _selectSubscribe;

	public:
		void setup(mref<PropertyOptions> options_);

	public:
		[[nodiscard]] ref<const PropertyModel> getModel() const noexcept;

		[[nodiscard]] ref<const PropertyView> getView() const noexcept;
	};

	/**/

	[[nodiscard]] extern UniquePtr<PropertyController> makeProperty(mref<PropertyOptions> options_);
}
