#pragma once

#include <Engine.Common/Types.hpp>

namespace clockwork::__internal::gfx::material {

    struct MaterialMeta {
        u16 albedo;
        u16 normal;
        u16 metalness;
        u16 roughness;
        u16 ao;
        u16 alpha;
    };

}
