#include "SfxMaterial.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id SfxMaterial::type_id = "SfxMaterial"_typeId;

type_id SfxMaterial::get_typeId() const noexcept {
    return _typeId;
}

asset_guid SfxMaterial::get_guid() const noexcept {
    return _guid;
}
