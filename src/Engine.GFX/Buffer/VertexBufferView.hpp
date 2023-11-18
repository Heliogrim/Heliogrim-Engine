#pragma once

#include <Engine.Common/Types.hpp>
#include "../API/__vkFwd.hpp"

namespace hg::engine::gfx {
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
