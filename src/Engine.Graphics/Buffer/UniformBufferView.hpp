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
        nmpt<const BufferLikeObject> _blo;

    public:
        [[nodiscard]] nmpt<const BufferLikeObject> object() const noexcept;

        void storeBuffer(mref<nmpt<const BufferLikeObject>> object_) noexcept;
    };
}
