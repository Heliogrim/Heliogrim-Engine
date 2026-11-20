#pragma once

#include "../CompileNode.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::graph {
    class CompileBinNode :
        public CompileNode {
    public:
        [[nodiscard]] uptr<RuntimeNode> compile(cref<CompilePassContext> ctx_) const noexcept override;
    };
}
