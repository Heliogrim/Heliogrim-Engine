#pragma once
#include "../Symbol/SymbolContext.hpp"

namespace hg::engine::gfx::render::graph {
    class ResolvePassContext {
    public:
        using this_type = ResolvePassContext;

    private:
        SymbolContext _symbolContext;

    public:
        [[nodiscard]] ref<SymbolContext> symbols() const noexcept;
    };
}
