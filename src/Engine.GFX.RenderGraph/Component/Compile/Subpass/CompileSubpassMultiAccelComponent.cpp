#include "CompileSubpassMultiAccelComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

CompileSubpassMultiAccelComponent::~CompileSubpassMultiAccelComponent() noexcept = default;

_STD span<const smr<const engine::gfx::acc::AccelerationEffect>>
CompileSubpassMultiAccelComponent::getAccelEffects() const noexcept {
    return _STD span {
        _accelerations.begin(), _accelerations.size()
    };
}

bool CompileSubpassMultiAccelComponent::hasAccelEffect(
    mref<smr<const acc::AccelerationEffect>> effect_
) const noexcept {
    return _STD ranges::find(_accelerations, effect_) != _accelerations.end();
}

void CompileSubpassMultiAccelComponent::addAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_) {
    _accelerations.push_back(_STD move(effect_));
}

void CompileSubpassMultiAccelComponent::removeAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_) {

    const auto it = _STD ranges::find(_accelerations, effect_);
    if (it != _accelerations.end()) {
        _accelerations.erase(it);
    }
}
