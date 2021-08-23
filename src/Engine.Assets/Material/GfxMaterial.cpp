#include "GfxMaterial.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id GfxMaterial::type_id = "GfxMaterial"_typeId;

type_id GfxMaterial::get_typeId() const noexcept {
    return _typeId;
}

asset_guid GfxMaterial::get_guid() const noexcept {
    return _guid;
}
