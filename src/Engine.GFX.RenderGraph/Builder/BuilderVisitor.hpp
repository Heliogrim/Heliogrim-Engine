#pragma once

#include <functional>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "../Visitor/Visitor.hpp"

namespace hg::engine::gfx::render::graph {
    enum class BuilderPredicateMode : bool {
        eSingle = false,
        eMulti = true
    };

    class BuilderVisitor :
        public Visitor {
    public:
        using this_type = BuilderVisitor;

        using predicate_type = _STD function<bool(cref<Node>)>;

    public:
        BuilderVisitor(
            mref<predicate_type> predicate_,
            mref<smr<Node>> begin_,
            mref<smr<Node>> end_,
            BuilderPredicateMode predicateMode_ = BuilderPredicateMode::eSingle
        ) noexcept;

        ~BuilderVisitor() noexcept override;

    private:
        BuilderPredicateMode _predicateMode;

        predicate_type _predicate;

        smr<Node> _begin;
        smr<Node> _end;

    private:
        smr<Node> _current;
        Vector<smr<Node>> _successor;

    public:
        [[nodiscard]] cref<smr<Node>> getCurrentNode() const noexcept;

        [[nodiscard]] cref<Vector<smr<Node>>> getSuccessorNodes() const noexcept;

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
