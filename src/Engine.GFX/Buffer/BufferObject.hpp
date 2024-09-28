#pragma once

#include "BufferLikeObject.hpp"

namespace hg::engine::gfx {
    class macro_novtable BufferObject :
        public InheritMeta<BufferObject, BufferLikeObject> {
    public:
        using this_type = BufferObject;

    public:
        constexpr BufferObject() noexcept = default;

        constexpr ~BufferObject() noexcept override = default;
    };
}
