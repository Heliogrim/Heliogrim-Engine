#pragma once
#include <Engine.Common/Make.hpp>

#include "../CompileNode.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileBinNode :
        public CompileNode {
    public:
        [[nodiscard]] uptr<RuntimeNode> compile(cref<CompilePassContext> ctx_) const noexcept override;
    };
}
