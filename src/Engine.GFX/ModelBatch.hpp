#pragma once

#include <Engine.Common/Math/Transformation.hpp>
#include "Buffer/Buffer.hpp"

namespace ember::engine::gfx {

    class ModelBatch {
    public:
    private:
        // Transformation...
        math::Transformation _transformation;

        // Geometry Data...
        VertexBuffer vertices;
        IndexBuffer indices;

        u32 indexCount;
        u32 indexOffset;

        // Bindings...
    };
}
