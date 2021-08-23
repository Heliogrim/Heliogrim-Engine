#include "Texture.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id Texture::type_id = "Texture"_typeId;

type_id Texture::get_typeId() const noexcept {
    return _typeId;
}

asset_guid Texture::get_guid() const noexcept {
    return _guid;
}
