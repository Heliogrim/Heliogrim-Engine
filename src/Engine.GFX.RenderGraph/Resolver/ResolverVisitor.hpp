#pragma once

#include "../Visitor/Visitor.hpp"

namespace hg::engine::gfx::render::graph {
    class ResolverVisitor :
        public Visitor {
    public:
        using this_type = ResolverVisitor;

    public:
        ~ResolverVisitor() noexcept override;

    public:
        void operator()(cref<Node> node_) override;
    };
}
