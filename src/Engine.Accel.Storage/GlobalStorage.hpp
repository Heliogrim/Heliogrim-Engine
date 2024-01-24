#pragma once

#include <set>
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
        std::set<Permutation> _permutations;
        DenseMap<HashedState<Stage>, smr<const Stage>> _stages;
        DenseMap<HashedState<AccelerationEffect>, smr<const AccelerationEffect>> _effects;
        DenseMap<HashedState<AccelerationPipeline>, smr<const AccelerationPipeline>> _pipelines;

    public:
        [[nodiscard]] bool hasAccelStage(
            cref<HashedState<Stage>> hashedState_
        ) noexcept override;

        [[nodiscard]] bool hasAccelStage(
            cref<smr<const Stage>> stage_
        ) noexcept override;

        [[nodiscard]] smr<const Stage> getAccelStage(
            cref<HashedState<Stage>> hashedState_
        ) noexcept override;

        [[nodiscard]] smr<const Stage> insertAccelStage(
            cref<HashedState<Stage>> hashedState_,
            mref<smr<const Stage>> stage_
        ) noexcept override;

    public:
        [[nodiscard]] bool hasAccelEffect(
            cref<HashedState<AccelerationEffect>> hashedState_
        ) noexcept override;

        [[nodiscard]] bool hasAccelEffect(
            cref<Guid> effectGuid_
        ) noexcept override;

        [[nodiscard]] bool hasAccelEffect(
            cref<smr<const AccelerationEffect>> effect_
        ) noexcept override;

        [[nodiscard]] smr<const AccelerationEffect> getAccelEffect(
            cref<Guid> effectGuid_
        ) noexcept override;

        [[nodiscard]] smr<const AccelerationEffect> getAccelEffect(
            cref<HashedState<AccelerationEffect>> hashedState_
        ) noexcept override;

        [[nodiscard]] smr<const AccelerationEffect> insertAccelEffect(
            cref<HashedState<AccelerationEffect>> hashedState_,
            mref<smr<const AccelerationEffect>> effect_
        ) noexcept override;

    public:
        [[nodiscard]] bool hasAccelPipeline(
            cref<HashedState<AccelerationPipeline>> hashedState_
        ) noexcept override;

        [[nodiscard]] bool hasAccelPipeline(
            cref<smr<const AccelerationPipeline>> pipeline_
        ) noexcept override;

        [[nodiscard]] smr<const AccelerationPipeline> getAccelPipeline(
            cref<HashedState<AccelerationPipeline>> hashedPipeline_
        ) noexcept override;

        [[nodiscard]] smr<const AccelerationPipeline> getAccelPipeline(
            cref<HashedState<Permutation>> hashedPermutation_,
            cref<HashedState<AccelerationEffect>> hashedEffect_
        ) noexcept override;

        [[nodiscard]] smr<const AccelerationPipeline> insertAccelPipeline(
            cref<Permutation> permutation_,
            mref<smr<const AccelerationPipeline>> pipeline_
        ) noexcept override;
    };
}
