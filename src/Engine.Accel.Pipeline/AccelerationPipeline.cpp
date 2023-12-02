#include "AccelerationPipeline.hpp"

#include <Engine.Accel.Effect/AccelerationEffect.hpp>

using namespace hg::engine::accel;
using namespace hg;

AccelerationPipeline::AccelerationPipeline(
    mref<smr<const AccelerationEffect>> effect_
) noexcept :
    _effect(_STD move(effect_)),
    _stages() {}

AccelerationPipeline::~AccelerationPipeline() = default;

smr<const AccelerationEffect> AccelerationPipeline::getEffect() const noexcept {
    return _effect;
}

cref<Vector<smr<StageDerivat>>> AccelerationPipeline::getStageDerivates() const noexcept {
    return _stages;
}

cref<BindLayout> AccelerationPipeline::getBindingLayout() const noexcept {
    return _bindLayout;
}

void AccelerationPipeline::setBindingLayout(mref<BindLayout> layout_) noexcept {
    _bindLayout = _STD move(layout_);
}
