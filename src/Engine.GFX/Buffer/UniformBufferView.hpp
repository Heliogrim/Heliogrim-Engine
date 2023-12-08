#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#include "BufferLikeObject.hpp"

namespace hg::engine::gfx {
    class UniformBufferView :
        public InheritBase<UniformBufferView> {
    public:
        using this_type = UniformBufferView;

    private:
        nmpt<BufferLikeObject> _blo;

    public:
        [[nodiscard]] nmpt<BufferLikeObject> object() const noexcept;

        void storeBuffer(mref<nmpt<BufferLikeObject>> object_) noexcept;
    };
}
