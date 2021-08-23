#include "PfxGeometryPrototype.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id PfxGeometryPrototype::type_id = "PfxGeometryPrototype"_typeId;

type_id PfxGeometryPrototype::get_typeId() const noexcept {
    return _typeId;
}

asset_guid PfxGeometryPrototype::get_guid() const noexcept {
    return _guid;
}
