#include "EntityPrototype.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id EntityPrototype::type_id = "EntityPrototype"_typeId;

type_id EntityPrototype::get_typeId() const noexcept {
    return _typeId;
}

asset_guid EntityPrototype::get_guid() const noexcept {
    return _guid;
}
