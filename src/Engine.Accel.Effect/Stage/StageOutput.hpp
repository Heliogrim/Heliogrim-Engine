#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Accel.Lang/Rep/Symbol.hpp>

namespace hg::engine::accel {
    struct StageOutput final {
        nmpt<const lang::Symbol> symbol;
    };
}
