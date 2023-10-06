#pragma once

#include "EffectCompiler.hpp"

namespace hg::engine::gfx::acc {
    [[nodiscard]] uptr<EffectCompiler> makeDxAccCompiler();
}
