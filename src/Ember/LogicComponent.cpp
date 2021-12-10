#include "LogicComponent.hpp"

using namespace ember;

LogicComponent::LogicComponent(mref<component_type_id> typeId_) noexcept :
    _typeId(_STD move(typeId_)) {}

cref<component_type_id> LogicComponent::typeId() const noexcept {
    return _typeId;
}
