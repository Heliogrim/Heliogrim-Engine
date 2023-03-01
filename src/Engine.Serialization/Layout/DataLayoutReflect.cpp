#include "DataLayoutReflect.hpp"

#include <cassert>

using namespace hg::engine::serialization;
using namespace hg;

DataLayoutReflect::DataLayoutReflect() :
    _ipc(nullptr),
    _ipd(nullptr),
    _rclass(nullptr) {}

DataLayoutReflect::~DataLayoutReflect() noexcept = default;

ptr<HeliogrimObject> DataLayoutReflect::instantiate() const {
    return _rclass->instantiate();
}

void DataLayoutReflect::destroy(mref<ptr<HeliogrimObject>> obj_) const {
    _rclass->destroy(_STD move(obj_));
}

void DataLayoutReflect::constructInPlace(const ptr<void> dst_) const {
    hasClass() ? _rclass->construct(dst_) : _ipc(dst_);
}

void DataLayoutReflect::destroyInPlace(const ptr<void> dst_) const {
    hasClass() ? _rclass->destruct(static_cast<const ptr<HeliogrimObject>>(dst_)) : _ipd(dst_);
}

bool DataLayoutReflect::hasClass() const noexcept {
    return _rclass != nullptr;
}

non_owning_rptr<HeliogrimClass> DataLayoutReflect::getClass() const noexcept {
    return _rclass;
}
