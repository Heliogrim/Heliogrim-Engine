#include "CompileSubpassSingleAccelComponent.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg;

CompileSubpassSingleAccelComponent::~CompileSubpassSingleAccelComponent() noexcept = default;

smr<const engine::gfx::acc::AccelerationEffect> CompileSubpassSingleAccelComponent::getAccelEffect() const noexcept {
    return clone(_acceleration);
}

void CompileSubpassSingleAccelComponent::setAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_) {
    _acceleration = _STD move(effect_);
}

void CompileSubpassSingleAccelComponent::clearAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_) {
    if (not effect_.empty() && _acceleration != effect_) {
        return;
    }

    _acceleration = nullptr;
}
