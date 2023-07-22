#include "BarrierComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

BarrierComponent::BarrierComponent(mref<string> barrierName_) noexcept :
    InheritMeta(),
    _name(_STD move(barrierName_)) {}

string_view BarrierComponent::getBarrierName() const noexcept {
    return _name;
}

void BarrierComponent::setBarrierName(string_view barrierName_) {
    _name = barrierName_;
}
