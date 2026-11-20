#pragma once

#include <Engine.Common/__macro.hpp>

#include "TextureLikeObject.hpp"

namespace hg::engine::graphics {
    class macro_novtable TextureObject :
        public InheritMeta<TextureObject, TextureLikeObject> {
    public:
        using this_type = TextureObject;

    public:
        constexpr TextureObject() noexcept = default;

        constexpr ~TextureObject() noexcept override = default;
    };
}
