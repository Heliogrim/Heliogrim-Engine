#pragma once

namespace hg::engine::gfx::acc {
    struct AccelerationGraphicsSpec;

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
}
