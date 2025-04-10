#include "ProviderComponent.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

cref<DenseSet<Provision>> ProviderComponent::getProvided() const noexcept {
    return _provided;
}

void ProviderComponent::setProvided(mref<DenseSet<Provision>> provided_) {
    _provided = std::move(provided_);
}
