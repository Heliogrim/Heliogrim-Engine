#include "EmberReflect.hpp"

using namespace ember;

EmberClass::EmberClass() = default;

cref<type_id> EmberClass::typeId() const noexcept {
    return _typeId;
}

ref<type_id> EmberClass::typeId() noexcept {
    return _typeId;
}
