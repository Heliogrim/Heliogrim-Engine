#pragma once

#include "TextureLikeObject.hpp"

namespace hg::engine::gfx {
    class macro_novtable TextureObject :
        public InheritMeta<TextureObject, TextureLikeObject> {
    public:
        using this_type = TextureObject;

    public:
        constexpr TextureObject() noexcept = default;

        constexpr ~TextureObject() noexcept override = default;
    };
}
