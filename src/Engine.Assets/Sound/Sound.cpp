#include "Sound.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id Sound::type_id = "Sound"_typeId;

type_id Sound::get_typeId() const noexcept {
    return _typeId;
}

asset_guid Sound::get_guid() const noexcept {
    return _guid;
}
