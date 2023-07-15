#include "SubpassSingleAccelComponent.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SubpassSingleAccelComponent::~SubpassSingleAccelComponent() noexcept = default;

smr<const engine::gfx::acc::AccelerationEffect> SubpassSingleAccelComponent::getAccelEffect() const noexcept {
    return clone(_acceleration);
}

void SubpassSingleAccelComponent::setAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_) {
    _acceleration = _STD move(effect_);
}

void SubpassSingleAccelComponent::clearAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_) {
    if (not effect_.empty() && _acceleration != effect_) {
        return;
    }

    _acceleration = nullptr;
}
