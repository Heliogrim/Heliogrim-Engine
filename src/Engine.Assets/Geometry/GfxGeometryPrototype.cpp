#include "GfxGeometryPrototype.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id GfxGeometryPrototype::type_id = "GfxGeometryPrototype"_typeId;

type_id GfxGeometryPrototype::get_typeId() const noexcept {
    return _typeId;
}

asset_guid GfxGeometryPrototype::get_guid() const noexcept {
    return _guid;
}
