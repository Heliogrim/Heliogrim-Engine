#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/List.hpp>

#include "InputAttribute.hpp"
#include "InputRate.hpp"
#include "Geometry/Vertex.hpp"

namespace ember::engine::gfx {
    struct FixedPipelineInput {
        /**
         * Binding Id
         */
        u8 binding = 0ui8;

        /**
         * Input Rate
         */
        InputRate rate = InputRate::ePerVertex;

        /**
         * Data Size
         */
        u32 dataSize = sizeof(vertex);

        /**
         * Data Attributes / Fields
         */
        vector<InputAttribute> attributes;
    };
}
