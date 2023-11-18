#pragma once

#include "TextureLikeObject.hpp"

namespace hg::engine::gfx {
    class __declspec(novtable) TextureObject :
        public InheritMeta<TextureObject, TextureLikeObject> {
    public:
        using this_type = TextureObject;

    public:
        constexpr TextureObject() noexcept = default;

        constexpr ~TextureObject() noexcept override = default;
    };
}
