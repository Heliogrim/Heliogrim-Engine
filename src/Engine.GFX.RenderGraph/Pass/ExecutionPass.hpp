#pragma once

#include <cassert>
#include <Engine.Common/Collection/Queue.hpp>

#include "ExecutionPass.hpp"
#include "ExecutionPassContext.hpp"
#include "RenderGraphPass.hpp"
#include "Engine.GFX.RenderGraph/Node/RuntimeNode.hpp"
#include "Engine.GFX.RenderGraph/Visitor/Visitor.hpp"

namespace hg::engine::render::graph {
    class ExecutionPass final :
        public RenderGraphPass {
    public:
        using this_type = ExecutionPass;

    public:
        constexpr ExecutionPass(cref<SymbolContext> symbolContext_) noexcept :
            RenderGraphPass(),
            _context(symbolContext_) {}

        constexpr ~ExecutionPass() noexcept = default;

    private:
        ExecutionPassContext _context;

    public:
        class VolatileExecutionVisitor final :
            public Visitor {
        public:
            VolatileExecutionVisitor(const ptr<ExecutionPass> owner_) noexcept;

            ~VolatileExecutionVisitor() noexcept override;

        private:
            const ptr<ExecutionPass> _owner;

        private:
            Queue<nmpt<Node>> _backlog;

            void step();

            void unroll();

        public:
            void operator()(cref<Node> node_) override;

            void operator()(cref<CompileNode> node_) override {
                assert(false);
            }
        };

    public:
        [[nodiscard]] VolatileExecutionVisitor visitor() noexcept {
            return VolatileExecutionVisitor { this };
        }
    };
}
