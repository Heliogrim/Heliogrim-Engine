#pragma once

namespace hg::engine::gfx::acc {
    enum class DataInputRate {
        ePerInvocation,
        ePerInstantiation,
        ePerPassInvocation,
        ePerPassInstantiation
    };
}
