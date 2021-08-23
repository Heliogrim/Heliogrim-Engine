#include "SfxMaterialPrototype.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id SfxMaterialPrototype::type_id = "SfxMaterialPrototype"_typeId;

type_id SfxMaterialPrototype::get_typeId() const noexcept {
    return _typeId;
}

asset_guid SfxMaterialPrototype::get_guid() const noexcept {
    return _guid;
}
