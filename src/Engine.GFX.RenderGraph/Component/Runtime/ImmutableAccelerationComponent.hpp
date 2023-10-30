#pragma once
#include "AccelerationComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class ImmutableAccelerationComponent final :
        public InheritMeta<ImmutableAccelerationComponent, AccelerationComponent> {
    public:
        using this_type = ImmutableAccelerationComponent;

    public:
        ImmutableAccelerationComponent() noexcept = delete;

        ImmutableAccelerationComponent(
            mref<Vector<smr<const acc::Symbol>>> passOutputSymbols_,
            mref<Vector<smr<const acc::Symbol>>> passInputSymbols_
        );

        constexpr ~ImmutableAccelerationComponent() noexcept override = default;

    private:
        Vector<smr<const acc::Symbol>> _passOutputSymbols;
        Vector<smr<const acc::Symbol>> _passInputSymbols;

        stage_type _passStages;

    public:
        [[nodiscard]] PassInvalidationResult iterate(cref<IterationPassContext> ctx_) override;

        [[nodiscard]] cref<stage_type> getStageEnumeration() const noexcept override;
    };
}
