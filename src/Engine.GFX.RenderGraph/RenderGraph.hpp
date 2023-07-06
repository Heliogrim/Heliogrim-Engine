#pragma once

#include <memory>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Node/AnchorNode.hpp"

#include "Relation/Provision.hpp"
#include "Relation/Requirement.hpp"

#include "Visitor/MaterialVisitor.hpp"

namespace hg::engine::gfx::render {
    class RenderGraph {
    public:
        using this_type = RenderGraph;

    private:
        _STD allocator<graph::Node> _alloc;
        mpt<graph::AnchorNode> _begin;
        mpt<graph::AnchorNode> _end;

    public:
        [[nodiscard]] nmpt<const graph::AnchorNode> begin() const noexcept;

        [[nodiscard]] nmpt<const graph::AnchorNode> end() const noexcept;

    public:
        ref<this_type> pushNode(nmpt<const graph::Node> where_, _In_ mref<smr<graph::Node>> node_);

        ref<this_type> pullNode(_In_ mref<nmpt<const graph::Node>> node_);

    public:
        void update(cref<graph::Visitor> visitor_);

    public:
        [[nodiscard]] Vector<graph::Provision> aggregateProvisions() const noexcept;

        [[nodiscard]] Vector<graph::Requirement> aggregateRequirements() const noexcept;
    };
}
