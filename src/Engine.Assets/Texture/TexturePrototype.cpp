#include "TexturePrototype.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id TexturePrototype::type_id = "TexturePrototype"_typeId;

type_id TexturePrototype::get_typeId() const noexcept {
    return _typeId;
}

asset_guid TexturePrototype::get_guid() const noexcept {
    return _guid;
}
