#pragma once
#include "../Symbol/SymbolContext.hpp"

namespace hg::engine::render::graph {
    class ResolvePassContext {
    public:
        using this_type = ResolvePassContext;

    public:
        constexpr ResolvePassContext(ref<SymbolContext> symbolContext_) noexcept :
            _symbolContext(symbolContext_) {}

        ResolvePassContext(mref<this_type>) = delete;

        ResolvePassContext(cref<this_type>) = delete;

    private:
        ref<SymbolContext> _symbolContext;

    public:
        [[nodiscard]] ref<SymbolContext> symbols() const noexcept;
    };
}
