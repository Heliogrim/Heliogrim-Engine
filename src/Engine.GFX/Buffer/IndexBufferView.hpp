#pragma once

#include <Engine.Common/Types.hpp>

#include "../API/__vkFwd.hpp"

namespace hg::engine::gfx {
    enum class VertexIndexType {
        eU16 = sizeof(u16),
        eU32 = sizeof(u32)
    };

    class IndexBufferView {
    public:
        using this_type = IndexBufferView;

        using size_type = streamsize;
        using offset_type = streamoff;

    public:
        VertexIndexType vertexIndexType;
        size_type size;
        offset_type offset;

        _::VkBuffer buffer;
    };
}
