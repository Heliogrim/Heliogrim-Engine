#include "BarrierComponent.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

BarrierComponent::BarrierComponent(mref<string> barrierName_) noexcept :
    InheritMeta(),
    _name(std::move(barrierName_)) {}

string_view BarrierComponent::getBarrierName() const noexcept {
    return _name;
}

void BarrierComponent::setBarrierName(string_view barrierName_) {
    _name = barrierName_;
}
