#include "PfxMaterialPrototype.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id PfxMaterialPrototype::type_id = "PfxMaterialPrototype"_typeId;

type_id PfxMaterialPrototype::get_typeId() const noexcept {
    return _typeId;
}

asset_guid PfxMaterialPrototype::get_guid() const noexcept {
    return _guid;
}
