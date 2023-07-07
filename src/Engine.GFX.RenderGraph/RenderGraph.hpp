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
        smr<graph::AnchorNode> _begin;
        smr<graph::AnchorNode> _end;

    public:
        RenderGraph();

        RenderGraph(mref<smr<graph::AnchorNode>> begin_, mref<smr<graph::AnchorNode>> end_) noexcept;

        RenderGraph(mref<this_type>) noexcept = delete;

        RenderGraph(cref<this_type>) = delete;

        ~RenderGraph() = default;

    public:
        [[nodiscard]] nmpt<const graph::AnchorNode> begin() const noexcept;

        [[nodiscard]] smr<graph::AnchorNode> begin();

        [[nodiscard]] nmpt<const graph::AnchorNode> end() const noexcept;

        [[nodiscard]] smr<graph::AnchorNode> end();

    public:
        void update(ref<graph::Visitor> visitor_);

    public:
        [[nodiscard]] Vector<graph::Provision> aggregateProvisions() const noexcept;

        [[nodiscard]] Vector<graph::Requirement> aggregateRequirements() const noexcept;
    };
}
