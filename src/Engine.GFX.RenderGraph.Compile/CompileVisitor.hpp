#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.RenderGraph/Visitor/Visitor.hpp>

namespace hg::engine::gfx::render::graph {
    class CompileVisitor final :
        public Visitor {
    public:
        using this_type = CompileVisitor;

    public:
        CompileVisitor(mref<nmpt<RuntimeGraph>> target_) noexcept;

        ~CompileVisitor() noexcept override = default;

    private:
        nmpt<RuntimeGraph> _target;

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

    public:
        void operator()(cref<CompileNode> node_) override;
    };
}
