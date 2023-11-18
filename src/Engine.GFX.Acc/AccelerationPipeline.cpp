#include "AccelerationPipeline.hpp"

using namespace hg::engine::gfx::acc;
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

cref<AccelerationPipelineBindings> AccelerationPipeline::getPipelineBindings() const noexcept {
    return _bindings;
}

void AccelerationPipeline::setPassBindings(mref<AccelerationPipelineBindings> bindings_) {
    _bindings = _STD move(bindings_);
}
