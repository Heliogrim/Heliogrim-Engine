#pragma once

#include "EffectCompiler.hpp"

namespace hg::engine::accel {
    [[nodiscard]] uptr<EffectCompiler> makeDxAccCompiler();
}
