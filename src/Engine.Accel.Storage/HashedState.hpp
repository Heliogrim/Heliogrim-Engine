#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Hash.hpp>

namespace hg::engine::accel {
    class AccelerationEffect;
    class AccelerationPipeline;
    class EffectProfile;
    class EffectSpecification;
    class Stage;
    struct Permutation;
}

namespace hg::engine::accel {
    template <typename Type_>
    struct HashedState {
        size_t value;
    };

    template <typename Type_>
    inline bool operator==(cref<HashedState<Type_>> rhs_, cref<HashedState<Type_>> lhs_) noexcept {
        return rhs_.value == lhs_.value;
    }
}

namespace hg::engine::accel {
    constexpr HashedState<Permutation> calcStorageHash(cref<Permutation> value_) noexcept {
        return {};
    }

    constexpr HashedState<AccelerationEffect> calcStorageHash(
        cref<AccelerationEffect> value_
    ) noexcept {
        return {};
    }

    constexpr HashedState<AccelerationPipeline> calcStorageHash(
        cref<AccelerationPipeline> value_
    ) noexcept {
        return {};
    }

    constexpr HashedState<EffectProfile> calcStorageHash(cref<EffectProfile> value_) noexcept {
        return {};
    }

    constexpr HashedState<EffectSpecification> calcStorageHash(
        cref<EffectSpecification> value_
    ) noexcept {
        return {};
    }

    constexpr HashedState<Stage> calcStorageHash(cref<Stage> value_) noexcept {
        return {};
    }

    constexpr HashedState<AccelerationPipeline> combineStorageHash(
        cref<HashedState<Permutation>> hashedPermutation_,
        cref<HashedState<AccelerationEffect>> hashedEffect_
    ) noexcept {

        size_t combined = 16538761uLL;
        hash::hashCombine(combined, hashedEffect_.value, hashedPermutation_.value);

        return HashedState<AccelerationPipeline> { combined };
    }
}
