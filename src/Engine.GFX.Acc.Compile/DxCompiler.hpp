#pragma once

#include "Compiler.hpp"

namespace hg::engine::gfx::acc {
    [[nodiscard]] uptr<Compiler> makeDxAccCompiler();
}
