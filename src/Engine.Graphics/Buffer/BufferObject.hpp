#pragma once

#include <Engine.Common/__macro.hpp>

#include "BufferLikeObject.hpp"

namespace hg::engine::graphics {
    class macro_novtable BufferObject :
        public InheritMeta<BufferObject, BufferLikeObject> {
    public:
        using this_type = BufferObject;

    public:
        constexpr BufferObject() noexcept = default;

        constexpr ~BufferObject() noexcept override = default;
    };
}
