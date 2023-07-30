#pragma once

#include <memory>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Node/AnchorNode.hpp"

#include "Relation/Provision.hpp"
#include "Relation/Requirement.hpp"

#include "Visitor/MaterialVisitor.hpp"

namespace hg::engine::gfx::render::graph {
    class RuntimeGraph {
    public:
        using this_type = RuntimeGraph;

    private:
        _STD allocator<Node> _alloc;
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
