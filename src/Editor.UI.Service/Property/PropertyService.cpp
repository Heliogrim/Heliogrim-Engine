#include "PropertyService.hpp"

#include <Engine.Reflect/Meta/MetaClass.hpp>

using namespace hg::editor::ui::service;
using namespace hg::editor::ui;
using namespace hg;

PropertyService::PropertyService() = default;

PropertyService::~PropertyService() = default;

void PropertyService::registerProperty(mref<Property> property_) {
	_registered.emplace_back(make_uptr<Property>(::hg::move(property_)));
}

Vector<nmpt<const Property>> PropertyService::fetchPropertiesFor(nmpt<const Actor> actor_) const {

	const auto actorClass = actor_->getMetaClass();
	const auto actorClassId = actorClass->typeId();

	auto result = Vector<nmpt<const Property>> {};
	for (const auto& property : _registered) {
		if (property->selector.is<nmpt<const ActorClass>>()) {
			if (property->selector.as<nmpt<const ActorClass>>()->exact(actorClassId)) {
				result.emplace_back(property.get());
			}
			continue;
		}

		if (property->selector.is<Property::selector_fn>()) {
			if (property->selector.as<Property::selector_fn>()(actor_)) {
				result.emplace_back(property.get());
			}
			continue;
		}
	}

	return result;
}

Vector<nmpt<const Property>> PropertyService::fetchPropertiesFor(nmpt<const HierarchyComponent> component_) const {

	const auto componentTypeId = component_->getTypeId();

	auto result = Vector<nmpt<const Property>> {};
	for (const auto& property : _registered) {
		if (property->selector.is<ComponentTypeId>()) {
			if (property->selector.as<ComponentTypeId>() == componentTypeId) {
				result.emplace_back(property.get());
			}
			continue;
		}

		if (property->selector.is<Property::selector_fn>()) {
			if (property->selector.as<Property::selector_fn>()(component_)) {
				result.emplace_back(property.get());
			}
			continue;
		}
	}

	return result;

}