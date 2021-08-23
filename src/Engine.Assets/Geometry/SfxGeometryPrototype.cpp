#include "SfxGeometryPrototype.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id SfxGeometryPrototype::type_id = "SfxGeometryPrototype"_typeId;

type_id SfxGeometryPrototype::get_typeId() const noexcept {
    return _typeId;
}

asset_guid SfxGeometryPrototype::get_guid() const noexcept {
    return _guid;
}
