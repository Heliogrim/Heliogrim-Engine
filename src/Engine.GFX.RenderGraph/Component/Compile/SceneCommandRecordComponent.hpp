#pragma once

#include "CommandRecordComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileSceneCommandRecordComponent final :
        public InheritMeta<CompileSceneCommandRecordComponent, CompileCommandRecordComponent> {
    public:
        using this_type = CompileSceneCommandRecordComponent;

    public:
        [[nodiscard]] uptr<CommandRecordComponent> compile(cref<CompilePassContext> ctx_) const noexcept override;
    };
}