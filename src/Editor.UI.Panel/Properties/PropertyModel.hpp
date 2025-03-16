#pragma once

#include <Editor.UI.Main/EditorUiModel.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

namespace hg::editor::ui {
	class PropertyView;
}

namespace hg::editor::ui::service {
	struct Property;
	class PropertyService;
}

namespace hg::editor::ui {
	class PropertyModel {
	public:
		using selection_type = typename decltype(EditorUiModel::Scene::selected)::holder_type::type;

	public:
		PropertyModel(
			ref<PropertyView> view_,
			SharedPtr<service::PropertyService> service_
		);

		~PropertyModel();

	private:
		ref<PropertyView> _view;

	public:
		SharedPtr<service::PropertyService> service;
		Vector<nmpt<const service::Property>> properties;

	public:
		void changeSubject(ref<const Variant<std::nullptr_t, nmpt<Actor>, nmpt<HierarchyComponent>>> subject_);
	};

	/**/

	[[nodiscard]] extern UniquePtr<PropertyModel> makePropertyModel(
		_Inout_ ref<PropertyView> view_,
		SharedPtr<service::PropertyService> service_
	);
}
