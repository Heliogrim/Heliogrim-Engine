#include "EmberReflect.hpp"

using namespace ember;

const ptr<const EmberClass> EmberObject::getClass() const noexcept {
    return _class;
}

EmberClass::EmberClass() = default;

cref<type_id> EmberClass::typeId() const noexcept {
    return _typeId;
}

ref<type_id> EmberClass::typeId() noexcept {
    return _typeId;
}
