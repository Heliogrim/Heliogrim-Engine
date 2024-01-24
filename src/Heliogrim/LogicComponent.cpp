#include "LogicComponent.hpp"

using namespace hg;

LogicComponent::LogicComponent(mref<component_type_id> typeId_) noexcept :
    _typeId(std::move(typeId_)) {}

cref<component_type_id> LogicComponent::getTypeId() const noexcept {
    return _typeId;
}
