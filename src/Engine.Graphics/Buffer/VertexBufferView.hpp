#pragma once

#include "../API/__vkFwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    class VertexBufferView {
    public:
        using this_type = VertexBufferView;

        using size_type = streamsize;
        using offset_type = streamoff;

    public:
        size_t stride;
        size_type size;
        offset_type offset;

        _::VkBuffer buffer;
    };
}
