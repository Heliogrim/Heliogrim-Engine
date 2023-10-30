#include "ImmutableAccelerationComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

ImmutableAccelerationComponent::ImmutableAccelerationComponent(
    mref<Vector<smr<const acc::Symbol>>> passOutputSymbols_,
    mref<Vector<smr<const acc::Symbol>>> passInputSymbols_
) :
    _passOutputSymbols(_STD move(passOutputSymbols_)),
    _passInputSymbols(_STD move(passInputSymbols_)),
    _passStages() {}

PassInvalidationResult ImmutableAccelerationComponent::iterate(cref<IterationPassContext> ctx_) {

    const auto allocated = true;
    if (allocated) {
        return PassInvalidationResult::eNever;
    }

    return PassInvalidationResult::eAlways;
}

cref<AccelerationComponent::stage_type> ImmutableAccelerationComponent::getStageEnumeration() const noexcept {
    return _passStages;
}
