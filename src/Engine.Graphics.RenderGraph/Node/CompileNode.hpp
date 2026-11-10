#pragma once

#include "Node.hpp"
#include "RuntimeNode.hpp"

namespace hg::engine::render::graph {
    class CompilePassContext;
}

namespace hg::engine::render::graph {
    class macro_novtable CompileNode :
        public Node {
    public:
        using this_type = CompileNode;

    protected:
        CompileNode() noexcept = default;

        CompileNode(mref<smr<Auxiliary>> auxiliary_) noexcept;

    public:
        ~CompileNode() noexcept override = default;

    public:
        [[nodiscard]] virtual uptr<RuntimeNode> compile(cref<CompilePassContext> ctx_) const noexcept = 0;
    };
}
