#include "DataLayoutReflect.hpp"

#include <cassert>

using namespace ember::engine::serialization;
using namespace ember;

DataLayoutReflect::DataLayoutReflect() :
    _ipc(nullptr),
    _ipd(nullptr) {}

DataLayoutReflect::~DataLayoutReflect() noexcept = default;

void DataLayoutReflect::constructInPlace(const ptr<void> dst_) {
    assert(_ipc);
    _ipc(dst_);
}

void DataLayoutReflect::destroyInPlace(const ptr<void> dst_) {
    assert(_ipd);
    _ipd(dst_);
}
