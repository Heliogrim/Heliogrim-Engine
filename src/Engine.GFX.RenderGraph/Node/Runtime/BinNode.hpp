#pragma once
#include "ConvergeNode.hpp"

namespace hg::engine::gfx::render::graph {
    class BinNode final :
        public ConvergeNode {
    public:
        using this_type = ConvergeNode;

    public:
        BinNode() noexcept = default;

        ~BinNode() noexcept override = default;

    public:
        [[nodiscard]] IterationResult iterate(cref<IterationPassContext> ctx_) noexcept override;

        [[nodiscard]] IterationResult resolve(cref<ResolvePassContext> ctx_) noexcept override;

        [[nodiscard]] IterationResult execute(cref<ExecutionPassContext> ctx_) noexcept override;
    };
}
