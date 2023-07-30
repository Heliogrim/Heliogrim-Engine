#pragma once

#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Queue.hpp>
#include <Engine.GFX.RenderGraph/Visitor/Visitor.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::gfx::render {
    class CompileVisitor final :
        public graph::Visitor {
    public:
        using this_type = CompileVisitor;

        using mapping_type = DenseMap<
            nmpt<const graph::Node>,
            _STD pair<nmpt<const graph::Node>, nmpt<const graph::Node>>
        >;

    public:
        CompileVisitor(mref<nmpt<graph::RuntimeGraph>> target_, mref<mapping_type> mapping_ = {}) noexcept;

        ~CompileVisitor() override;

    private:
        nmpt<graph::RuntimeGraph> _target;
        mapping_type _mapping;

        Queue<ptr<const graph::Node>> _backlog;
        nmpt<const graph::Node> _scope;

    private:
        void streamUnwind();

    public:
        void operator()(cref<graph::Node> node_) override;

    public:
        void operator()(cref<graph::AnchorNode> node_) override;

        void operator()(cref<graph::BarrierNode> node_) override;

        void operator()(cref<graph::ConvergeNode> node_) override;

        void operator()(cref<graph::DivergeNode> node_) override;

        void operator()(cref<graph::SelectorNode> node_) override;

        void operator()(cref<graph::ProviderNode> node_) override;

        void operator()(cref<graph::SubpassNode> node_) override;

    public:
        void operator()(cref<graph::CompileNode> node_) override;

    public:
        void operator()(cref<graph::CompileSubpassNode> node_) override;

    private:
        _STD pair<smr<graph::Node>, smr<graph::Node>> constructSingleAccel(
            nmpt<graph::CompileSubpassComponent> csc_,
            nmpt<graph::CompileSubpassSingleAccelComponent> cac_
        );

        _STD pair<smr<graph::Node>, smr<graph::Node>> constructMultiAccel(
            nmpt<graph::CompileSubpassComponent> csc_,
            nmpt<graph::CompileSubpassMultiAccelComponent> cac_
        );

        _STD pair<smr<graph::Node>, smr<graph::Node>> constructMaterialAccel(
            nmpt<graph::CompileSubpassComponent> csc_,
            nmpt<graph::CompileSubpassMaterialAccelComponent> cac_
        );
    };
}
