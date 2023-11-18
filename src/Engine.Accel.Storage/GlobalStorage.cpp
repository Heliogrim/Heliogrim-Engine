#include "GlobalStorage.hpp"

#include <Engine.GFX.Acc/Stage/Stage.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/AccelerationPipeline.hpp>

#include <algorithm>

using namespace hg::engine::gfx::acc;
using namespace hg::engine::accel;
using namespace hg;

bool GlobalStorage::hasAccelStage(cref<HashedState<Stage>> hashedState_) noexcept {
    return _stages.contains(hashedState_);
}

bool GlobalStorage::hasAccelStage(cref<smr<const Stage>> stage_) noexcept {
    return _STD ranges::contains(
        _stages.values(),
        stage_,
        [](const auto& value_) {
            return value_.second;
        }
    );
}

smr<const Stage> GlobalStorage::getAccelStage(cref<HashedState<Stage>> hashedState_) noexcept {
    const auto iter = _stages.find(hashedState_);
    return iter != _stages.end() ? iter->second : nullptr;
}

smr<const Stage> GlobalStorage::insertAccelStage(
    cref<HashedState<Stage>> hashedState_,
    mref<smr<const Stage>> stage_
) noexcept {
    const auto iter = _stages.emplace(hashedState_, _STD move(stage_));
    return iter.first->second;
}

bool GlobalStorage::hasAccelEffect(cref<HashedState<AccelerationEffect>> hashedState_) noexcept {
    return _effects.contains(hashedState_);
}

bool GlobalStorage::hasAccelEffect(cref<Guid> effectGuid_) noexcept {
    return _STD ranges::contains(
        _effects.values(),
        effectGuid_,
        [](const auto& value_) {
            return value_.second->getGuid();
        }
    );
}

bool GlobalStorage::hasAccelEffect(cref<smr<const gfx::acc::AccelerationEffect>> effect_) noexcept {
    return _STD ranges::contains(
        _effects.values(),
        effect_,
        [](const auto& value_) {
            return value_.second;
        }
    );
}

smr<const AccelerationEffect> GlobalStorage::getAccelEffect(cref<Guid> effectGuid_) noexcept {
    const auto iter = _STD ranges::find(
        _effects.values(),
        effectGuid_,
        [](const auto& value_) {
            return value_.second->getGuid();
        }
    );
    return iter != _effects.values().end() ? iter->second : nullptr;
}

smr<const AccelerationEffect> GlobalStorage::getAccelEffect(
    cref<HashedState<AccelerationEffect>> hashedState_
) noexcept {
    const auto iter = _effects.find(hashedState_);
    return iter != _effects.end() ? iter->second : nullptr;
}

smr<const AccelerationEffect> GlobalStorage::insertAccelEffect(
    cref<HashedState<AccelerationEffect>> hashedState_,
    mref<smr<const AccelerationEffect>> effect_
) noexcept {
    const auto iter = _effects.emplace(hashedState_, _STD move(effect_));
    return iter.first->second;
}

bool GlobalStorage::hasAccelPipeline(cref<HashedState<AccelerationPipeline>> hashedState_) noexcept {
    return _pipelines.contains(hashedState_);
}

bool GlobalStorage::hasAccelPipeline(cref<smr<const AccelerationPipeline>> pipeline_) noexcept {
    return _STD ranges::contains(
        _pipelines.values(),
        pipeline_,
        [](const auto& value_) {
            return value_.second;
        }
    );
}

smr<const AccelerationPipeline> GlobalStorage::getAccelPipeline(
    cref<HashedState<AccelerationPipeline>> hashedPipeline_
) noexcept {
    const auto iter = _pipelines.find(hashedPipeline_);
    return iter != _pipelines.end() ? iter->second : nullptr;
}

smr<const AccelerationPipeline> GlobalStorage::getAccelPipeline(
    cref<HashedState<Permutation>> hashedPermutation_,
    cref<HashedState<AccelerationEffect>> hashedEffect_
) noexcept {
    return getAccelPipeline(combineStorageHash(hashedPermutation_, hashedEffect_));
}

smr<const AccelerationPipeline> GlobalStorage::insertAccelPipeline(
    cref<Permutation> permutation_,
    mref<smr<const AccelerationPipeline>> pipeline_
) noexcept {
    return nullptr;
}
