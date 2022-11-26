#include "EmberReflect.hpp"

#ifdef _DEBUG
#include <Engine.Logging/Logger.hpp>
#endif

using namespace ember;

EmberClass::EmberClass():
    EmberStruct(),
    _typeId(),
    _rnctor(nullptr),
    _rndtor(nullptr),
    _rctor(nullptr),
    _rdtor(nullptr) {}

cref<type_id> EmberClass::typeId() const noexcept {
    return _typeId;
}

ref<type_id> EmberClass::typeId() noexcept {
    return _typeId;
}

ptr<EmberObject> EmberClass::instantiate() const {
    #ifdef _DEBUG
    if (_rnctor == nullptr) {
        IM_CORE_ERROR("Tried to instantiate a not reconstructable class type.");
        return nullptr;
    }
    #endif
    return static_cast<ptr<EmberObject>>(_rnctor());
}

void EmberClass::destroy(mref<ptr<EmberObject>> obj_) const {
    #ifdef _DEBUG
    if (_rndtor == nullptr) {
        IM_CORE_ERROR("Tried to destroy not destroyable ember object by class type.");
        // Warning: This should actually never happen, cause we are required to hold a public destructor for every public type definition
        __debugbreak();
        return;
    }
    #endif
    _rndtor(_STD move(obj_));
}

ptr<EmberObject> EmberClass::construct(ptr<void> dst_) const {
    #ifdef _DEBUG
    if (_rctor == nullptr) {
        IM_CORE_ERROR("Tried to construct a not reconstructable class type.");
        return nullptr;
    }
    #endif
    return _rctor(dst_);
}

void EmberClass::destruct(const ptr<EmberObject> obj_) const {
    #ifdef _DEBUG
    if (_rdtor == nullptr) {
        IM_CORE_ERROR("Tried to destruct not destructable ember object by class type.");
        return;
    }
    #endif
    _rdtor(obj_);
}
