#pragma once
#include "RenderGraphPass.hpp"

#include "IterationPassContext.hpp"
#include "../Node/RuntimeNode.hpp"
#include "../Visitor/Visitor.hpp"

namespace hg::engine::render::graph {
    class IterationPass :
        public RenderGraphPass {
    public:
        friend class VolatileIterationVisitor;

    public:
        using this_type = IterationPass;

    public:
        IterationPass(ref<SymbolContext> symbolContext_) noexcept :
            RenderGraphPass(),
            _context(symbolContext_) {}

        ~IterationPass() noexcept = default;

    private:
        IterationPassContext _context;

    public:
        class VolatileIterationVisitor final :
            public graph::Visitor {
        private:
            const ptr<IterationPass> _owner;

        public:
            constexpr VolatileIterationVisitor(const ptr<IterationPass> owner_) noexcept:
                _owner(owner_) {}

            constexpr ~VolatileIterationVisitor() noexcept = default;

        public:
            void operator()(cref<Node> node_) override {
                const auto invalidation = const_cast<ref<RuntimeNode>>(static_cast<cref<RuntimeNode>>(node_)).iterate(
                    _owner->_context
                );
                node_.traverse(*this);
            }

            void operator()(cref<CompileNode> node_) override {
                assert(false);
            }
        };

    public:
        [[nodiscard]] VolatileIterationVisitor visitor() noexcept {
            return VolatileIterationVisitor { this };
        }
    };
}
