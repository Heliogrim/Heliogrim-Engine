#include "MaterialPrototypeParameter.hpp"

using namespace hg::engine::gfx::material;
using namespace hg;

MaterialPrototypeParameter::MaterialPrototypeParameter(
    mref<ParameterIdentifier> identifier_,
    mref<String> name_,
    mref<accel::TransferDataType> dataType_,
    mref<uptr<MaterialParameterStorageBase>> defaultStorage_
) noexcept :
    _identifier(_STD move(identifier_)),
    _name(_STD move(name_)),
    _dataType(_STD move(dataType_)),
    _defaultStorage(_STD move(defaultStorage_)) {}

MaterialPrototypeParameter::~MaterialPrototypeParameter() noexcept = default;

ref<MaterialPrototypeParameter::this_type> MaterialPrototypeParameter::operator=(mref<this_type> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _identifier = _STD exchange(other_._identifier, ParameterIdentifier { static_cast<u16>(~0u) });
        _name = _STD move(other_._name);
        _dataType = _STD move(other_._dataType);
        _defaultStorage = _STD move(other_._defaultStorage);
    }

    return *this;
}

ParameterIdentifier MaterialPrototypeParameter::getId() const noexcept {
    return _identifier;
}

StringView MaterialPrototypeParameter::getName() const noexcept {
    return _name;
}

engine::accel::TransferDataType MaterialPrototypeParameter::getDataType() const noexcept {
    return _dataType;
}

nmpt<const MetaClass> MaterialPrototypeParameter::getStorageClass() const noexcept {
    if (_defaultStorage) {
        return _defaultStorage->getMetaClass();
    }
    return {};
}
