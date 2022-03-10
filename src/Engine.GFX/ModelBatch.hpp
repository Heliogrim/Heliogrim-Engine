#pragma once

#include <Engine.Common/Math/Transform.hpp>
#include "Buffer/Buffer.hpp"

namespace ember::engine::gfx {

    struct ModelBatch {
        // Transform...
        math::Transform transform;

        // Geometry Data...
        VertexBuffer vertices;
        IndexBuffer indices;

        u32 indexCount;
        u32 indexOffset;

        // Bindings...
    };
}
