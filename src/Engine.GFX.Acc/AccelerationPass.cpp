#include "AccelerationPass.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

AccelerationPass::AccelerationPass(
    mref<smr<AccelerationEffect>> effect_
) noexcept :
    HeliogrimObject(),
    _effect(_STD move(effect_)),
    _stages() {}

AccelerationPass::~AccelerationPass() = default;

smr<AccelerationEffect> AccelerationPass::getEffect() const noexcept {
    return _effect;
}

cref<Vector<smr<AccelerationStageDerivat>>> AccelerationPass::getStageDerivates() const noexcept {
    return _stages;
}

cref<AccelerationPassBindings> AccelerationPass::getPassBindings() const noexcept {
    return _bindings;
}

void AccelerationPass::setPassBindings(mref<AccelerationPassBindings> bindings_) {
    _bindings = _STD move(bindings_);
}
