#pragma once

#include "../API/__vkFwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
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
