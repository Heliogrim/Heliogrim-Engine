#include "PropertyModel.hpp"

#include "PropertyView.hpp"
#include "Editor.UI.Service/Property/PropertyService.hpp"

using namespace hg::editor::ui;
using namespace hg;

PropertyModel::PropertyModel(ref<PropertyView> view_, SharedPtr<service::PropertyService> service_) :
	_view(view_),
	service(::hg::move(service_)),
	properties() {}

PropertyModel::~PropertyModel() = default;

void PropertyModel::changeSubject(ref<const Variant<std::nullptr_t, nmpt<Actor>, nmpt<HierarchyComponent>>> subject_) {

	if (subject_.is<std::nullptr_t>()) {
		properties.clear();
		_view.displayProperties(properties, {});
		return;

	}

	if (subject_.is<nmpt<Actor>>()) {
		properties = service->fetchPropertiesFor(subject_.as<nmpt<Actor>>());
		_view.displayProperties(properties, subject_.as<nmpt<Actor>>());
		return;

	}

	if (subject_.is<nmpt<HierarchyComponent>>()) {
		properties = service->fetchPropertiesFor(subject_.as<nmpt<HierarchyComponent>>());
		_view.displayProperties(properties, subject_.as<nmpt<HierarchyComponent>>());
	}
}

UniquePtr<PropertyModel> editor::ui::makePropertyModel(ref<PropertyView> view_, SharedPtr<service::PropertyService> service_) {
	return make_uptr<PropertyModel>(view_, ::hg::move(service_));
}
