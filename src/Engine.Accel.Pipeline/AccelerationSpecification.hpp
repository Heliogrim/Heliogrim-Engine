#pragma once

namespace hg::engine::accel {
    struct GraphicsSpec;

    /**/

    enum class DepthCompareOp {
        // ~ Discard all
        eNever = 0x0,
        // ~ Override all
        eAlways = 0x7,
        /**/
        eLess = 0x1,
        eLessEqual = 0x3,
        eEqual = 0x2,
        eGreaterEqual = 0x6,
        eGreater = 0x4
    };

    enum class StencilCompareOp {
        // ~ Never happy path
        eNever = 0x0,
        // ~ Always happy path
        eAlways = 0x7
    };

    enum class StencilOp {
        eKeep = 0x0,
        eReplace = 0x2
    };

    /**/

    enum class PrimitiveTopology {
        eTriangleList = 0x03,
        ePatchList = 0x0A
    };
}
