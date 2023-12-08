#pragma once
#include <memory>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "Node/Runtime/AnchorNode.hpp"
#include "Node/Node.hpp"
#include "Relation/Provision.hpp"
#include "Relation/Requirement.hpp"

namespace hg::engine::render::graph {
    class CompileGraph {
    public:
        using this_type = CompileGraph;

    private:
        _STD allocator<Node> _alloc;
        smr<AnchorNode> _begin;
        smr<AnchorNode> _end;

    public:
        CompileGraph();

        CompileGraph(mref<smr<AnchorNode>> begin_, mref<smr<AnchorNode>> end_) noexcept;

        CompileGraph(mref<this_type>) noexcept = delete;

        CompileGraph(cref<this_type>) = delete;

        ~CompileGraph() = default;

    public:
        [[nodiscard]] nmpt<const AnchorNode> begin() const noexcept;

        [[nodiscard]] smr<AnchorNode> begin();

        [[nodiscard]] nmpt<const AnchorNode> end() const noexcept;

        [[nodiscard]] smr<AnchorNode> end();

    public:
        void update(ref<Visitor> visitor_);

        void reverse(ref<Visitor> visitor_);

    private:
        DenseSet<Provision> _expectedProvision;
        DenseSet<Requirement> _expectedRequirement;

    public:
        bool addExpectedProvision(mref<Provision> expected_);

        bool addExpectedRequirement(mref<Requirement> expected_);

    public:
        [[nodiscard]] _STD span<const Provision> expectedProvision() const noexcept;

        [[nodiscard, deprecated]] _STD span<const Requirement> expectedRequirement() const noexcept;
    };
}
