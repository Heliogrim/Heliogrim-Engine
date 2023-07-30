#pragma once

#include "../Node.hpp"

namespace hg::engine::gfx::render::graph {
    class __declspec(novtable) CompileNode :
        public Node {
    public:
        using this_type = CompileNode;

    protected:
        CompileNode() noexcept = default;

        CompileNode(mref<smr<Auxiliary>> auxiliary_) noexcept;

    public:
        ~CompileNode() noexcept override = default;
    };
}
