#pragma once

#include "../Symbol/SymbolContext.hpp"

namespace hg::engine::render::graph {
    class ExecutionPassContext {
    public:
        using this_type = ExecutionPassContext;

    public:
        constexpr ExecutionPassContext(cref<SymbolContext> symbolContext_) noexcept :
            _symbolContext(symbolContext_) {}

        ExecutionPassContext(mref<this_type>) = delete;

        ExecutionPassContext(cref<this_type>) = delete;

    private:
        cref<SymbolContext> _symbolContext;

    public:
        [[nodiscard]] cref<SymbolContext> symbols() const noexcept;
    };
}
