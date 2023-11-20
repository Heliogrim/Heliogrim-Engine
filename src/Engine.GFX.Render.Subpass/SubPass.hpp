#pragma once
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::gfx::render::graph {
    class ScopedSymbolContext;
}

namespace hg::engine::gfx::render {
    class SubPass {
    public:
        using this_type = SubPass;

    public:
        constexpr SubPass() noexcept = default;

        constexpr virtual ~SubPass() noexcept = default;

    public:
        virtual void setup(ref<graph::ScopedSymbolContext> symCtx_) noexcept;

        virtual void destroy() noexcept;

    public:
        virtual void declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept;

        virtual void declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept;

        virtual void declareInputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept;

    public:
        virtual void iterate() noexcept = 0;

        virtual void resolve() noexcept = 0;

        virtual void execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept = 0;
    };
}
