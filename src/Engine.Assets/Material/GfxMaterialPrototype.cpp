#include "GfxMaterialPrototype.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id GfxMaterialPrototype::type_id = "GfxMaterialPrototype"_typeId;

type_id GfxMaterialPrototype::get_typeId() const noexcept {
    return _typeId;
}

asset_guid GfxMaterialPrototype::get_guid() const noexcept {
    return _guid;
}
