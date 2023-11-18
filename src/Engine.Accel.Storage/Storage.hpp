#pragma once
#include <span>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "HashedState.hpp"

namespace hg::engine::gfx::acc {
    class EffectProfile;
    class EffectSpecification;
    class AccelerationPipeline;
    class Stage;
    class AccelerationEffect;
}

namespace hg::engine::accel {
    struct Permutation {
        nmpt<const gfx::acc::EffectProfile> profile;
        nmpt<const gfx::acc::EffectSpecification> specification;
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
            cref<HashedState<gfx::acc::Stage>> hashedState_
        ) noexcept = 0;

        [[nodiscard]] virtual bool hasAccelStage(
            cref<smr<const gfx::acc::Stage>> stage_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const gfx::acc::Stage> getAccelStage(
            cref<HashedState<gfx::acc::Stage>> hashedState_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const gfx::acc::Stage> insertAccelStage(
            cref<HashedState<gfx::acc::Stage>> hashedState_,
            mref<smr<const gfx::acc::Stage>> stage_
        ) noexcept = 0;

    public:
        [[nodiscard]] virtual bool hasAccelEffect(
            cref<HashedState<gfx::acc::AccelerationEffect>> hashedState_
        ) noexcept = 0;

        [[nodiscard]] virtual bool hasAccelEffect(
            cref<Guid> effectGuid_
        ) noexcept = 0;

        [[nodiscard]] virtual bool hasAccelEffect(
            cref<smr<const gfx::acc::AccelerationEffect>> effect_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const gfx::acc::AccelerationEffect> getAccelEffect(
            cref<Guid> effectGuid_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const gfx::acc::AccelerationEffect> getAccelEffect(
            cref<HashedState<gfx::acc::AccelerationEffect>> hashedState_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const gfx::acc::AccelerationEffect> insertAccelEffect(
            cref<HashedState<gfx::acc::AccelerationEffect>> hashedState_,
            mref<smr<const gfx::acc::AccelerationEffect>> effect_
        ) noexcept = 0;

    public:
        [[nodiscard]] virtual bool hasAccelPipeline(
            cref<HashedState<gfx::acc::AccelerationPipeline>> hashedState_
        ) noexcept = 0;

        [[nodiscard]] virtual bool hasAccelPipeline(
            cref<smr<const gfx::acc::AccelerationPipeline>> pipeline_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const gfx::acc::AccelerationPipeline> getAccelPipeline(
            cref<HashedState<gfx::acc::AccelerationPipeline>> hashedPipeline_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const gfx::acc::AccelerationPipeline> getAccelPipeline(
            cref<HashedState<Permutation>> hashedPermutation_,
            cref<HashedState<gfx::acc::AccelerationEffect>> hashedEffect_
        ) noexcept = 0;

        [[nodiscard]] virtual smr<const gfx::acc::AccelerationPipeline> insertAccelPipeline(
            cref<Permutation> permutation_,
            mref<smr<const gfx::acc::AccelerationPipeline>> pipeline_
        ) noexcept = 0;
    };
}
