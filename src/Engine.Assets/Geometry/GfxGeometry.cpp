#include "GfxGeometry.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id GfxGeometry::type_id = "GfxGeometry"_typeId;

type_id GfxGeometry::get_typeId() const noexcept {
    return _typeId;
}

asset_guid GfxGeometry::get_guid() const noexcept {
    return _guid;
}
