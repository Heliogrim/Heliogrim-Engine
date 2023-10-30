#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc/Symbol/Symbol.hpp>

namespace hg::engine::gfx::render::graph {
    struct SymbolizedResource {
        smr<const acc::Symbol> symbol;
        void* data;
    };
}
