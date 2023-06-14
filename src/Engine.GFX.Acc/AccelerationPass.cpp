#include "AccelerationPass.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

AccelerationPass::AccelerationPass() noexcept = default;

AccelerationPass::AccelerationPass(
    mref<smr<AccelerationEffect>> effect_,
    mref<Vector<smr<AccelerationStageDerivat>>> stages_
) noexcept :
    _effect(_STD move(effect_)),
    _stages(_STD move(stages_)) {}

AccelerationPass::~AccelerationPass() = default;

smr<AccelerationEffect> AccelerationPass::getEffect() const noexcept {
    return _effect;
}

cref<Vector<smr<AccelerationStageDerivat>>> AccelerationPass::getStageDerivates() const noexcept {
    return _stages;
}
