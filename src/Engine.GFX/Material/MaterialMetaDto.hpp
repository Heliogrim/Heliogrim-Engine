#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::gfx::experimental {
    struct MaterialMetaDto final {
        u32 diffuse;
        u32 normal;
        u32 roughness;
        u32 metalness;
        u32 ao;
        u32 alpha;
    };
}
