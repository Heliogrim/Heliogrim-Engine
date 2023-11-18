#pragma once

#include "../CompileNode.hpp"
#include "../Runtime/SubpassNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/SubPassNode.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileSubPassNode :
        public CompileNode {
    public:
        using this_type = CompileSubPassNode;

    public:
        CompileSubPassNode() noexcept;

        ~CompileSubPassNode() noexcept override = default;

    public:
        void accept(ref<Visitor> visitor_) const override;

        void traverse(ref<Visitor> visitor_) const override;

        void rtraverse(ref<Visitor> visitor_) const override;

    private:
        smr<Node> _next;
        nmpt<const Node> _prev;

    public:
        void setNext(mref<smr<Node>> next_);

        [[nodiscard]] smr<Node> getNext() const noexcept;

        void setPrev(mref<nmpt<const Node>> prev_);

        [[nodiscard]] nmpt<const Node> getPrev() const noexcept;

    public:
        /**/
    private:
        _STD function<uptr<SubPassNodeBase>(cref<CompilePassContext> ctx_)> _subPassBuilder;

    public:
        void setSubPassBuilder(mref<decltype(_subPassBuilder)> subPassBuilder_) noexcept;

    public:
        [[nodiscard]] uptr<RuntimeNode> compile(cref<CompilePassContext> ctx_) const noexcept override;
    };
}
