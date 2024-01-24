#include "DataLayoutReflect.hpp"

#include <cassert>

using namespace hg::engine::serialization;
using namespace hg;

DataLayoutReflect::DataLayoutReflect() :
    _ipc(nullptr),
    _ipd(nullptr),
    _rclass(nullptr) {}

DataLayoutReflect::~DataLayoutReflect() noexcept = default;

ptr<ClassMetaBase> DataLayoutReflect::instantiate() const {
    // TODO:
    // WARNING:
    // ERROR:
    //return _rclass->instantiate();
    return nullptr;
}

void DataLayoutReflect::destroy(mref<ptr<ClassMetaBase>> obj_) const {
    // TODO:
    // WARNING:
    // ERROR:
    //_rclass->destroy(std::move(obj_));
}

void DataLayoutReflect::constructInPlace(const ptr<void> dst_) const {
    // TODO:
    // WARNING:
    // ERROR:
    //hasClass() ? _rclass->construct(dst_) : _ipc(dst_);
    _ipc(dst_);
}

void DataLayoutReflect::destroyInPlace(const ptr<void> dst_) const {
    // TODO:
    // WARNING:
    // ERROR:
    //hasClass() ? _rclass->destruct(static_cast<const ptr<ClassMetaBase>>(dst_)) : _ipd(dst_);
    _ipd(dst_);
}

bool DataLayoutReflect::hasClass() const noexcept {
    return _rclass != nullptr;
}

non_owning_rptr<const MetaClass> DataLayoutReflect::getClass() const noexcept {
    return _rclass;
}
