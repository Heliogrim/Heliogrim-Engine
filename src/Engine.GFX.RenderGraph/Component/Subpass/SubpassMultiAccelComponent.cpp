#include "SubpassMultiAccelComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SubpassMultiAccelComponent::~SubpassMultiAccelComponent() noexcept = default;

Vector<smr<engine::gfx::acc::Symbol>> SubpassMultiAccelComponent::aggregateImportedSymbols() const {
    return {};
}

Vector<smr<engine::gfx::acc::Symbol>> SubpassMultiAccelComponent::aggregateExportedSymbols() const {
    return {};
}

_STD span<const smr<const engine::gfx::acc::AccelerationEffect>>
SubpassMultiAccelComponent::getAccelEffects() const noexcept {
    return _STD span {
        _accelerations.begin(), _accelerations.size()
    };
}

bool SubpassMultiAccelComponent::hasAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_) const noexcept {
    return _STD ranges::find(_accelerations, effect_) != _accelerations.end();
}

void SubpassMultiAccelComponent::addAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_) {
    _accelerations.push_back(_STD move(effect_));
}

void SubpassMultiAccelComponent::removeAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_) {

    const auto it = _STD ranges::find(_accelerations, effect_);
    if (it != _accelerations.end()) {
        _accelerations.erase(it);
    }
}
