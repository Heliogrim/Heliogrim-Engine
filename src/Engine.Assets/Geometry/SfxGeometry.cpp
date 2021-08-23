#include "SfxGeometry.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id SfxGeometry::type_id = "SfxGeometry"_typeId;

type_id SfxGeometry::get_typeId() const noexcept {
    return _typeId;
}

asset_guid SfxGeometry::get_guid() const noexcept {
    return _guid;
}
