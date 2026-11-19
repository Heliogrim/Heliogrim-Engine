#pragma once

#include <Engine.Common/__macro.hpp>

#include "../Pass/ExecutionPassContext.hpp"
#include "../Pass/IterationPassContext.hpp"
#include "../Pass/ResolvePassContext.hpp"
#include "Node.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::graph {
    enum class IterationResultBits {
        eNone = 0x0,
        eInvalidateState = 0x1 << 0,
        eInvalidateResources = 0x1 << 1
    };

    typedef Flag<IterationResultBits> IterationResult;

    class macro_novtable RuntimeNode :
        public Node {
    protected:
        RuntimeNode() noexcept;

        RuntimeNode(mref<smr<Auxiliary>> auxiliary_) noexcept;

    public:
        [[nodiscard]] virtual IterationResult iterate(cref<IterationPassContext> ctx_) noexcept = 0;

        [[nodiscard]] virtual IterationResult resolve(cref<ResolvePassContext> ctx_) noexcept = 0;

        [[nodiscard]] virtual IterationResult execute(cref<ExecutionPassContext> ctx_) noexcept = 0;
    };

    template <typename NodeType_>
    concept IsRuntimeNode = std::derived_from<NodeType_, RuntimeNode>;
}
