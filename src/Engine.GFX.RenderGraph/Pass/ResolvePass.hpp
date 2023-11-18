#pragma once
#include "RenderGraphPass.hpp"
#include "ResolvePassContext.hpp"
#include "../Visitor/Visitor.hpp"
#include "../Node/RuntimeNode.hpp"

namespace hg::engine::gfx::render::graph {
    class ResolvePass final :
        public RenderGraphPass {
    public:
        friend class VolatileResolveVisitor;

    public:
        using this_type = ResolvePass;

    public:
        constexpr ResolvePass(ref<SymbolContext> symbolContext_) noexcept :
            RenderGraphPass(),
            _context(symbolContext_) {}

        constexpr ~ResolvePass() noexcept = default;

    private:
        ResolvePassContext _context;

    public:
        class VolatileResolveVisitor final :
            public Visitor {
        public:
            using this_type = VolatileResolveVisitor;

        public:
            constexpr VolatileResolveVisitor(const ptr<ResolvePass> owner_) noexcept :
                _owner(owner_) {}

            constexpr ~VolatileResolveVisitor() noexcept = default;

        private:
            const ptr<ResolvePass> _owner;

        public:
            void operator()(cref<Node> node_) override {
                const auto invalidation = const_cast<ref<RuntimeNode>>(static_cast<cref<RuntimeNode>>(node_)).resolve(
                    _owner->_context
                );
                node_.traverse(*this);
            }

            void operator()(cref<CompileNode> node_) override {
                assert(false);
            }
        };

    public:
        [[nodiscard]] VolatileResolveVisitor visitor() noexcept {
            return VolatileResolveVisitor { this };
        }
    };
}
