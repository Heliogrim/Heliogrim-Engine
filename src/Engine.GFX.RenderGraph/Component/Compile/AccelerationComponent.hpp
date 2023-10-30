#pragma once
#include "../CompileComponent.hpp"
#include "../../Pass/CompilePassContext.hpp"
#include "../Runtime/AccelerationComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class __declspec(novtable) CompileAccelerationComponent :
        public InheritMeta<CompileAccelerationComponent, CompileComponent> {
    public:
        using this_type = CompileAccelerationComponent;

    public:
        [[nodiscard]] virtual uptr<AccelerationComponent> compile(cref<CompilePassContext> ctx_) const noexcept = 0;
    };
}
