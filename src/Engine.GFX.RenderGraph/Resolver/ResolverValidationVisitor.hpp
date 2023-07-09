#pragma once

#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Collection/Stack.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "ResolverVisitor.hpp"
#include "../Relation/Provision.hpp"

namespace hg::engine::gfx::render::graph {
    class ResolverValidationVisitor final :
        public ResolverVisitor {
    public:
        using this_type = ResolverValidationVisitor;

    public:
        ResolverValidationVisitor() noexcept;

        ~ResolverValidationVisitor() override;

    private:
        Stack<nmpt<const Node>> _lastDiverge;
        Stack<nmpt<const Node>> _lastConverge;

        Vector<nmpt<const DenseSet<Provision>>> _providers;

    public:
        void operator()(cref<Node> node_) override;

    public:
        void operator()(cref<AnchorNode> node_) override;

        void operator()(cref<BarrierNode> node_) override;

        void operator()(cref<ConvergeNode> node_) override;

        void operator()(cref<DivergeNode> node_) override;

        void operator()(cref<SelectorNode> node_) override;

        void operator()(cref<ProviderNode> node_) override;

        void operator()(cref<SubpassNode> node_) override;
    };
}
