#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::gfx::experimental {

    struct MaterialMetaDto final {
        u16 diffuse;
        u16 normal;
        u16 roughness;
        u16 ao;
    };
}
