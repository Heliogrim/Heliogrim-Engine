#pragma once
#include "AccelerationComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileImmutableAccelerationComponent final :
        public InheritMeta<CompileImmutableAccelerationComponent, CompileAccelerationComponent> {
    public:
        using this_type = CompileImmutableAccelerationComponent;

    private:
        Vector<smr<const acc::Symbol>> _passOutputSymbols;
        Vector<smr<const acc::Symbol>> _passInputSymbols;

    public:
        [[nodiscard]] uptr<AccelerationComponent> compile(cref<CompilePassContext> ctx_) const noexcept override;
    };
}
