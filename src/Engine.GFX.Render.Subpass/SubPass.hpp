#pragma once
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::gfx::render::graph {
    class SymbolContext;
}

namespace hg::engine::gfx::render {
    class SubPass {
    public:
        using this_type = SubPass;

    public:
        constexpr SubPass() noexcept = default;

        constexpr virtual ~SubPass() noexcept = default;

    public:
        virtual void declareOutputs(ref<graph::SymbolContext> symCtx_) noexcept;

        virtual void declareTransforms(ref<graph::SymbolContext> symCtx_) noexcept;

        virtual void declareInputs(ref<graph::SymbolContext> symCtx_) noexcept;

    public:
        virtual void iterate() noexcept = 0;

        virtual void resolve() noexcept = 0;

        virtual void execute(cref<graph::SymbolContext> symCtx_) noexcept = 0;
    };
}
