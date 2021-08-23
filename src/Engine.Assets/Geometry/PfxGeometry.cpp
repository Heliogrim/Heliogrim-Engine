#include "PfxGeometry.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id PfxGeometry::type_id = "PfxGeometry"_typeId;

type_id PfxGeometry::get_typeId() const noexcept {
    return _typeId;
}

asset_guid PfxGeometry::get_guid() const noexcept {
    return _guid;
}
