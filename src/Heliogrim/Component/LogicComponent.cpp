#include "LogicComponent.hpp"

#include <Engine.Common/Move.hpp>

using namespace hg;

LogicComponent::LogicComponent(mref<ComponentGuid> guid_, mref<ComponentTypeId> typeId_) noexcept :
	_typeId(::hg::move(typeId_)),
	_guid(::hg::move(guid_)),
	_label() {}

cref<ComponentTypeId> LogicComponent::getTypeId() const noexcept {
	return _typeId;
}

ref<const ComponentGuid> LogicComponent::guid() const noexcept {
	return _guid;
}

ref<const ComponentLabel> LogicComponent::label() const noexcept {
	return _label;
}

void LogicComponent::setLabel(mref<ComponentLabel> label_) {
	_label = ::hg::move(label_);
}
