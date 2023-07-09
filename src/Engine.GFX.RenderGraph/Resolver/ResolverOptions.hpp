#pragma once
#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Types.hpp>

namespace hg::engine::gfx::render::graph {
    enum class ResolverPassFlagBits : u8 {
        eBasicLayout = 0b0000'0001,
        eOptimizeDiverging = 0b0001'0000,
        eEliminate = 0b0010'0000,
        /**/
        eValidate = 0b0000'1000,
    };

    typedef Flag<ResolverPassFlagBits> ResolverPassFlags;

    /**/

    struct ResolverOptions {
        ResolverPassFlags flags;
    };
}
