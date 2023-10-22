#pragma once
#include <Engine.Common/Types.hpp>

namespace hg::engine::gfx::render::pipeline {
    enum class IGProcessResult : u8 {
        eSucceeded = 0x0,
        eCached = 0x1,
        eBatched = 0x2,
        /**/
        eReusedCached = 0x21,
        eReusedBatched = 0x31,
        /**/
        eFailed = 0xFF
    };
}
