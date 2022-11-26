#include "DataLayoutReflect.hpp"

#include <cassert>

using namespace ember::engine::serialization;
using namespace ember;

DataLayoutReflect::DataLayoutReflect() :
    _ipc(nullptr),
    _ipd(nullptr),
    _rclass(nullptr) {}

DataLayoutReflect::~DataLayoutReflect() noexcept = default;

ptr<EmberObject> DataLayoutReflect::instantiate() const {
    return _rclass->instantiate();
}

void DataLayoutReflect::destroy(mref<ptr<EmberObject>> obj_) const {
    _rclass->destroy(_STD move(obj_));
}

void DataLayoutReflect::constructInPlace(const ptr<void> dst_) const {
    hasClass() ? _rclass->construct(dst_) : _ipc(dst_);
}

void DataLayoutReflect::destroyInPlace(const ptr<void> dst_) const {
    hasClass() ? _rclass->destruct(static_cast<const ptr<EmberObject>>(dst_)) : _ipd(dst_);
}

bool DataLayoutReflect::hasClass() const noexcept {
    return _rclass != nullptr;
}

non_owning_rptr<EmberClass> DataLayoutReflect::getClass() const noexcept {
    return _rclass;
}
