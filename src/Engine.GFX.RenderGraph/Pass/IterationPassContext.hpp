#pragma once
#include "../Symbol/SymbolContext.hpp"

namespace hg::engine::gfx::render::graph {
    class IterationPassContext {
    public:
        using this_type = IterationPassContext;

    private:
        SymbolContext _symbolContext;

    public:
        [[nodiscard]] ref<SymbolContext> symbols() const noexcept;
    };
}
