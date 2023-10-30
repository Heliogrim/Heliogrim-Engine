#pragma once
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.GFX.Acc/AccelerationPass.hpp>

#include "../RuntimeComponent.hpp"
#include "../../Pass/IterationPassContext.hpp"

namespace hg::engine::gfx::render::graph {
    enum class PassInvalidationResult {
        eNever,
        eAlways,
        /**/
        eNone,
        ePartialInvalidated,
        eInvalidated
    };

    class AccelerationComponent :
        public InheritMeta<AccelerationComponent, RuntimeComponent> {
    public:
        using this_type = AccelerationComponent;

        using set_type = InlineAutoArray<smr<const acc::AccelerationPass>>;
        using stage_type = InlineAutoArray<set_type>;

    public:
        [[nodiscard]] virtual PassInvalidationResult iterate(cref<IterationPassContext> ctx_) = 0;

    public:
        [[nodiscard]] virtual cref<stage_type> getStageEnumeration() const noexcept = 0;
    };
}
