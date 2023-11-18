#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Hash.hpp>

namespace hg::engine::accel {
    struct Permutation;
}

namespace hg::engine::gfx::acc {
    class AccelerationEffect;
    class AccelerationPipeline;
    class EffectProfile;
    class EffectSpecification;
    class Stage;
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

    constexpr HashedState<gfx::acc::AccelerationEffect> calcStorageHash(
        cref<gfx::acc::AccelerationEffect> value_
    ) noexcept {
        return {};
    }

    constexpr HashedState<gfx::acc::AccelerationPipeline> calcStorageHash(
        cref<gfx::acc::AccelerationPipeline> value_
    ) noexcept {
        return {};
    }

    constexpr HashedState<gfx::acc::EffectProfile> calcStorageHash(cref<gfx::acc::EffectProfile> value_) noexcept {
        return {};
    }

    constexpr HashedState<gfx::acc::EffectSpecification> calcStorageHash(
        cref<gfx::acc::EffectSpecification> value_
    ) noexcept {
        return {};
    }

    constexpr HashedState<gfx::acc::Stage> calcStorageHash(cref<gfx::acc::Stage> value_) noexcept {
        return {};
    }

    constexpr HashedState<gfx::acc::AccelerationPipeline> combineStorageHash(
        cref<HashedState<Permutation>> hashedPermutation_,
        cref<HashedState<gfx::acc::AccelerationEffect>> hashedEffect_
    ) noexcept {

        size_t combined = 16538761uLL;
        hash::hashCombine(combined, hashedEffect_.value, hashedPermutation_.value);

        return HashedState<gfx::acc::AccelerationPipeline> { combined };
    }
}
