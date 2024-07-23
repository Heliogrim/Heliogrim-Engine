#pragma once

#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "ModuleSource.hpp"

namespace hg::engine::accel {
    class macro_novtable ModuleBuilder {
    public:
        using this_type = ModuleBuilder;

    public:
        ModuleBuilder() noexcept = default;

        virtual ~ModuleBuilder() noexcept = default;

    public:
        [[nodiscard]] virtual uptr<ModuleSource> build(
            cref<smr<AccelerationPipeline>> targetPass_,
            cref<smr<const class EffectSpecification>> specifications_,
            cref<smr<class StageDerivat>> stage_,
            cref<uptr<ModuleSource>> previous_
        ) const = 0;
    };
}
