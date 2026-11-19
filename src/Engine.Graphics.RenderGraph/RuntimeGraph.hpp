#pragma once

#include <Engine.Common/__macro.hpp>

#include "Node/Runtime/AnchorNode.hpp"
#include "Relation/Provision.hpp"
#include "Relation/Requirement.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::graph {
    class RuntimeGraph {
    public:
        using this_type = RuntimeGraph;

    private:
        std::allocator<Node> _alloc;
        smr<AnchorNode> _begin;
        smr<AnchorNode> _end;

    public:
        RuntimeGraph();

        RuntimeGraph(mref<smr<AnchorNode>> begin_, mref<smr<AnchorNode>> end_) noexcept;

        RuntimeGraph(mref<this_type>) noexcept = delete;

        RuntimeGraph(cref<this_type>) = delete;

        ~RuntimeGraph() = default;

    public:
        [[nodiscard]] nmpt<const AnchorNode> begin() const noexcept;

        [[nodiscard]] smr<AnchorNode> begin();

        [[nodiscard]] nmpt<const AnchorNode> end() const noexcept;

        [[nodiscard]] smr<AnchorNode> end();

    public:
        void update(ref<Visitor> visitor_);

    public:
        [[nodiscard]] Vector<Provision> aggregateProvisions() const noexcept;

        [[nodiscard]] Vector<Requirement> aggregateRequirements() const noexcept;
    };
}
