#pragma once

#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg::engine::gfx {
    class __declspec(novtable) BufferLikeObject :
        public InheritBase<BufferLikeObject> {
    public:
        using this_type = BufferLikeObject;

    public:
        constexpr BufferLikeObject() noexcept = default;

        BufferLikeObject(cref<this_type>) noexcept = delete;

        BufferLikeObject(mref<this_type>) noexcept = delete;

        constexpr virtual ~BufferLikeObject() noexcept = default;

    public:
        ref<this_type> operator=(cref<this_type>) noexcept = delete;

        ref<this_type> operator=(mref<this_type>) noexcept = delete;
    };
}
