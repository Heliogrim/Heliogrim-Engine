#pragma once
#include "../CompileComponent.hpp"
#include "../../Pass/CompilePassContext.hpp"
#include "../Runtime/CommandRecordComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileCommandRecordComponent :
        public InheritMeta<CompileCommandRecordComponent, CompileComponent> {
    public:
        using this_type = CompileCommandRecordComponent;

    public:
        [[nodiscard]] virtual uptr<CommandRecordComponent> compile(cref<CompilePassContext> ctx_) const noexcept = 0;
    };
}
