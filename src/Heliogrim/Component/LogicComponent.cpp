#include "LogicComponent.hpp"

#include <Engine.Common/Move.hpp>

using namespace hg;

LogicComponent::LogicComponent(mref<ComponentGuid> guid_, mref<ComponentTypeId> typeId_) noexcept :
	_typeId(::hg::move(typeId_)),
	_guid(::hg::move(guid_)) {}

cref<ComponentTypeId> LogicComponent::getTypeId() const noexcept {
	return _typeId;
}

ref<const ComponentGuid> LogicComponent::getComponentGuid() const noexcept {
	return _guid;
}