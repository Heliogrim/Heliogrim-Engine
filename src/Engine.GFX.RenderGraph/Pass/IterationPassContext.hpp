#pragma once
#include "../Symbol/SymbolContext.hpp"

namespace hg::engine::gfx::render::graph {
    class IterationPassContext {
    public:
        using this_type = IterationPassContext;

    public:
        constexpr IterationPassContext(ref<SymbolContext> symbolContext_) noexcept :
            _symbolContext(symbolContext_) {}

        IterationPassContext(mref<this_type>) = delete;

        IterationPassContext(cref<this_type>) = delete;

        constexpr ~IterationPassContext() noexcept = default;

    private:
        ref<SymbolContext> _symbolContext;

    public:
        [[nodiscard]] ref<SymbolContext> symbols() const noexcept;
    };
}
