#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#include "BufferLikeObject.hpp"

namespace hg::engine::gfx {
    class StorageBufferView :
        public InheritBase<StorageBufferView> {
    public:
        using this_type = StorageBufferView;

    private:
        nmpt<BufferLikeObject> _blo;

    public:
        [[nodiscard]] nmpt<BufferLikeObject> object() const noexcept;
    };
}
