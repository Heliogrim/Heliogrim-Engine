#include "MaterialPrototypeParameter.hpp"

using namespace hg::engine::gfx::material;
using namespace hg;

MaterialPrototypeParameter::MaterialPrototypeParameter(
    mref<ParameterIdentifier> identifier_,
    mref<String> name_,
    mref<accel::TransferDataType> dataType_,
    mref<uptr<MaterialParameterStorageBase>> defaultStorage_
) noexcept :
    _identifier(std::move(identifier_)),
    _name(std::move(name_)),
    _dataType(std::move(dataType_)),
    _defaultStorage(std::move(defaultStorage_)) {}

MaterialPrototypeParameter::~MaterialPrototypeParameter() noexcept = default;

ref<MaterialPrototypeParameter::this_type> MaterialPrototypeParameter::operator=(mref<this_type> other_) noexcept {

    if (std::addressof(other_) != this) {
        _identifier = std::exchange(other_._identifier, ParameterIdentifier { static_cast<u16>(~0u) });
        _name = std::move(other_._name);
        _dataType = std::move(other_._dataType);
        _defaultStorage = std::move(other_._defaultStorage);
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
