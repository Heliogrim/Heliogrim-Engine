#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Acc/Symbol/Symbol.hpp>

#include "SymbolizedResource.hpp"

namespace hg::engine::gfx::render::graph {
    class SymbolContext {
    public:
        using this_type = SymbolContext;

    public:
        SymbolContext() noexcept = default;

        ~SymbolContext() noexcept = default;

    public:
        void exposeSymbol(mref<smr<const acc::Symbol>> symbol_, nmpt<SymbolizedResource> resource_);

        [[nodiscard]] nmpt<SymbolizedResource> exportSymbol(mref<smr<const acc::Symbol>> symbol_);

        [[nodiscard]] nmpt<SymbolizedResource> importSymbol(mref<smr<const acc::Symbol>> symbol_);
    };
}
