#pragma once
#include <span>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "HashedState.hpp"

namespace hg::engine::accel {
    class EffectProfile;
    class EffectSpecification;
    class AccelerationPipeline;
    class Stage;
    class AccelerationEffect;
}

namespace hg::engine::accel {
    struct Permutation {
        nmpt<const EffectProfile> profile;
        nmpt<const EffectSpecification> specification;
    };
}

namespace std {
    template <typename Type_>
    struct hash<::hg::engine::accel::HashedState<Type_>> {
        [[nodiscard]] constexpr size_t operator()(
            const ::hg::engine::accel::HashedState<Type_>& value_
        ) const noexcept {
            return value_.value;
        }
    };
}

namespace hg::engine::accel {
    class __declspec(novtable) AccelStorage {
    public:
        using this_type = AccelStorage;

    public:
        constexpr AccelStorage() noexcept = default;

        constexpr virtual ~AccelStorage() noexcept = default;

    public:
        [[nodiscard]] virtual bool hasAccelStage(
            cref<HashedState<Stage>> hashedState_
        ) noexcept = 0;

        [[nodiscard]] virtual bool hasAccelStage(
            cref<smr<const Stage>> stage_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const Stage> getAccelStage(
            cref<HashedState<Stage>> hashedState_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const Stage> insertAccelStage(
            cref<HashedState<Stage>> hashedState_,
            mref<smr<const Stage>> stage_
        ) noexcept = 0;

    public:
        [[nodiscard]] virtual bool hasAccelEffect(
            cref<HashedState<AccelerationEffect>> hashedState_
        ) noexcept = 0;

        [[nodiscard]] virtual bool hasAccelEffect(
            cref<Guid> effectGuid_
        ) noexcept = 0;

        [[nodiscard]] virtual bool hasAccelEffect(
            cref<smr<const AccelerationEffect>> effect_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const AccelerationEffect> getAccelEffect(
            cref<Guid> effectGuid_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const AccelerationEffect> getAccelEffect(
            cref<HashedState<AccelerationEffect>> hashedState_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const AccelerationEffect> insertAccelEffect(
            cref<HashedState<AccelerationEffect>> hashedState_,
            mref<smr<const AccelerationEffect>> effect_
        ) noexcept = 0;

    public:
        [[nodiscard]] virtual bool hasAccelPipeline(
            cref<HashedState<AccelerationPipeline>> hashedState_
        ) noexcept = 0;

        [[nodiscard]] virtual bool hasAccelPipeline(
            cref<smr<const AccelerationPipeline>> pipeline_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const AccelerationPipeline> getAccelPipeline(
            cref<HashedState<AccelerationPipeline>> hashedPipeline_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const AccelerationPipeline> getAccelPipeline(
            cref<HashedState<Permutation>> hashedPermutation_,
            cref<HashedState<AccelerationEffect>> hashedEffect_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const AccelerationPipeline> insertAccelPipeline(
            cref<Permutation> permutation_,
            mref<smr<const AccelerationPipeline>> pipeline_
        ) noexcept = 0;
    };
}
