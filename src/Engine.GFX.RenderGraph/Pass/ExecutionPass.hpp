#pragma once

#include <cassert>

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
            constexpr VolatileExecutionVisitor(const ptr<ExecutionPass> owner_) noexcept :
                _owner(owner_) {}

            constexpr ~VolatileExecutionVisitor() noexcept = default;

        private:
            const ptr<ExecutionPass> _owner;

        public:
            void operator()(cref<Node> node_) override {
                [[maybe_unused]] const auto invalidation = const_cast<ref<RuntimeNode>>(static_cast<cref<RuntimeNode>>(
                    node_)).execute(_owner->_context);
                node_.traverse(*this);
            }

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
