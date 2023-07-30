#include "MaterialPrototypeParameter.hpp"

using namespace hg::engine::gfx::material;
using namespace hg::engine::gfx::acc;
using namespace hg;

MaterialPrototypeParameter::MaterialPrototypeParameter(
    mref<string> uniqueName_,
    mref<TransferDataType> dataType_,
    mref<uptr<MaterialParameterStorageBase>> defaultStorage_
) noexcept :
    _uniqueName(_STD move(uniqueName_)),
    _dataType(_STD move(dataType_)),
    _defaultStorage(_STD move(defaultStorage_)) {}

MaterialPrototypeParameter::~MaterialPrototypeParameter() noexcept = default;

ref<MaterialPrototypeParameter::this_type> MaterialPrototypeParameter::operator=(mref<this_type> other_) {

    if (_STD addressof(other_) != this) {
        _uniqueName = _STD move(other_._uniqueName);
        _dataType = _STD move(other_._dataType);
        _defaultStorage = _STD move(other_._defaultStorage);
    }

    return *this;
}

string MaterialPrototypeParameter::getUniqueName() const noexcept {
    return _uniqueName;
}

TransferDataType MaterialPrototypeParameter::getDataType() const noexcept {
    return _dataType;
}

nmpt<const MetaClass> MaterialPrototypeParameter::getStorageClass() const noexcept {
    if (_defaultStorage) {
        return _defaultStorage->getMetaClass();
    }
    return {};
}
