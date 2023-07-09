#include "AnchorComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

AnchorComponent::AnchorComponent() noexcept = default;

AnchorComponent::AnchorComponent(mref<string> anchorName_) noexcept :
    Component(),
    _name(_STD move(anchorName_)) {}

string_view AnchorComponent::getAnchorName() const noexcept {
    return _name;
}
