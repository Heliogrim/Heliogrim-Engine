#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::accel {
    struct GraphicsSpec;

    /**/

    enum class FaceCulling : u8 {
        eNone = 0x0u,
        eFront = 0x1u,
        eBack = 0x2u,
        eBackAndFront = 0x3u
    };

    enum class FaceMode : u8 {
        eFill = 0x0u,
        eLine = 0x1u,
        ePoint = 0x2u
    };

    enum class FaceWinding : bool {
        eCcw = false,
        eCw = true
    };

    enum class DepthCompareOp : u8 {
        // ~ Discard all
        eNever = 0x0u,
        // ~ Override all
        eAlways = 0x7u,
        /**/
        eLess = 0x1u,
        eLessEqual = 0x3u,
        eEqual = 0x2u,
        eGreaterEqual = 0x6u,
        eGreater = 0x4u
    };

    enum class StencilCompareOp : u8 {
        // ~ Never happy path
        eNever = 0x0u,
        // ~ Always happy path
        eAlways = 0x7u
    };

    enum class StencilOp : u8 {
        eKeep = 0x0u,
        eReplace = 0x2u
    };

    /**/

    enum class PrimitiveTopology : u8 {
        eTriangleList = 0x03u,
        ePatchList = 0x0Au
    };
}
