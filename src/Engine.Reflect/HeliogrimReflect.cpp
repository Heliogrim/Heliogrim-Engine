#include "HeliogrimReflect.hpp"

#ifdef _DEBUG
#include <Engine.Logging/Logger.hpp>
#endif

using namespace hg;

HeliogrimClass::HeliogrimClass():
    HeliogrimStruct(),
    _typeId(),
    _rnctor(nullptr),
    _rndtor(nullptr),
    _rctor(nullptr),
    _rdtor(nullptr) {}

cref<type_id> HeliogrimClass::typeId() const noexcept {
    return _typeId;
}

ref<type_id> HeliogrimClass::typeId() noexcept {
    return _typeId;
}

ptr<HeliogrimObject> HeliogrimClass::instantiate() const {
    #ifdef _DEBUG
    if (_rnctor == nullptr) {
        IM_CORE_ERROR("Tried to instantiate a not reconstructable class type.");
        return nullptr;
    }
    #endif
    return static_cast<ptr<HeliogrimObject>>(_rnctor());
}

void HeliogrimClass::destroy(mref<ptr<HeliogrimObject>> obj_) const {
    #ifdef _DEBUG
    if (_rndtor == nullptr) {
        IM_CORE_ERROR("Tried to destroy not destroyable hg object by class type.");
        // Warning: This should actually never happen, cause we are required to hold a public destructor for every public type definition
        __debugbreak();
        return;
    }
    #endif
    _rndtor(_STD move(obj_));
}

ptr<HeliogrimObject> HeliogrimClass::construct(ptr<void> dst_) const {
    #ifdef _DEBUG
    if (_rctor == nullptr) {
        IM_CORE_ERROR("Tried to construct a not reconstructable class type.");
        return nullptr;
    }
    #endif
    return _rctor(dst_);
}

void HeliogrimClass::destruct(const ptr<HeliogrimObject> obj_) const {
    #ifdef _DEBUG
    if (_rdtor == nullptr) {
        IM_CORE_ERROR("Tried to destruct not destructable hg object by class type.");
        return;
    }
    #endif
    _rdtor(obj_);
}
