#include "PfxMaterial.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id PfxMaterial::type_id = "PfxMaterial"_typeId;

type_id PfxMaterial::get_typeId() const noexcept {
    return _typeId;
}

asset_guid PfxMaterial::get_guid() const noexcept {
    return _guid;
}
