#pragma once

#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>

#include "Storage.hpp"

namespace hg::engine::accel {
    class GlobalStorage final :
        public AccelStorage {
    public:
        using this_type = GlobalStorage;

    public:
        GlobalStorage() noexcept = default;

        ~GlobalStorage() noexcept override = default;

    private:
        DenseSet<Permutation> _permutations;
        DenseMap<HashedState<gfx::acc::Stage>, smr<const gfx::acc::Stage>> _stages;
        DenseMap<HashedState<gfx::acc::AccelerationEffect>, smr<const gfx::acc::AccelerationEffect>> _effects;
        DenseMap<HashedState<gfx::acc::AccelerationPipeline>, smr<const gfx::acc::AccelerationPipeline>> _pipelines;

    public:
        [[nodiscard]] bool hasAccelStage(
            cref<HashedState<gfx::acc::Stage>> hashedState_
        ) noexcept override;

        [[nodiscard]] bool hasAccelStage(
            cref<smr<const gfx::acc::Stage>> stage_
        ) noexcept override;

        [[nodiscard]] smr<const gfx::acc::Stage> getAccelStage(
            cref<HashedState<gfx::acc::Stage>> hashedState_
        ) noexcept override;

        [[nodiscard]] smr<const gfx::acc::Stage> insertAccelStage(
            cref<HashedState<gfx::acc::Stage>> hashedState_,
            mref<smr<const gfx::acc::Stage>> stage_
        ) noexcept override;

    public:
        [[nodiscard]] bool hasAccelEffect(
            cref<HashedState<gfx::acc::AccelerationEffect>> hashedState_
        ) noexcept override;

        [[nodiscard]] bool hasAccelEffect(
            cref<Guid> effectGuid_
        ) noexcept override;

        [[nodiscard]] bool hasAccelEffect(
            cref<smr<const gfx::acc::AccelerationEffect>> effect_
        ) noexcept override;

        [[nodiscard]] smr<const gfx::acc::AccelerationEffect> getAccelEffect(
            cref<Guid> effectGuid_
        ) noexcept override;

        [[nodiscard]] smr<const gfx::acc::AccelerationEffect> getAccelEffect(
            cref<HashedState<gfx::acc::AccelerationEffect>> hashedState_
        ) noexcept override;

        [[nodiscard]] smr<const gfx::acc::AccelerationEffect> insertAccelEffect(
            cref<HashedState<gfx::acc::AccelerationEffect>> hashedState_,
            mref<smr<const gfx::acc::AccelerationEffect>> effect_
        ) noexcept override;

    public:
        [[nodiscard]] bool hasAccelPipeline(
            cref<HashedState<gfx::acc::AccelerationPipeline>> hashedState_
        ) noexcept override;

        [[nodiscard]] bool hasAccelPipeline(
            cref<smr<const gfx::acc::AccelerationPipeline>> pipeline_
        ) noexcept override;

        [[nodiscard]] smr<const gfx::acc::AccelerationPipeline> getAccelPipeline(
            cref<HashedState<gfx::acc::AccelerationPipeline>> hashedPipeline_
        ) noexcept override;

        [[nodiscard]] smr<const gfx::acc::AccelerationPipeline> getAccelPipeline(
            cref<HashedState<Permutation>> hashedPermutation_,
            cref<HashedState<gfx::acc::AccelerationEffect>> hashedEffect_
        ) noexcept override;

        [[nodiscard]] smr<const gfx::acc::AccelerationPipeline> insertAccelPipeline(
            cref<Permutation> permutation_,
            mref<smr<const gfx::acc::AccelerationPipeline>> pipeline_
        ) noexcept override;
    };
}
