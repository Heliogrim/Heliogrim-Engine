#include "SoundPrototype.hpp"

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::assets;

type_id SoundPrototype::type_id = "SoundPrototype"_typeId;

type_id SoundPrototype::get_typeId() const noexcept {
    return _typeId;
}

asset_guid SoundPrototype::get_guid() const noexcept {
    return _guid;
}
