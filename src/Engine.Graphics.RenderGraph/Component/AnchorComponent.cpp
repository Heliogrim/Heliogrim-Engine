#include "AnchorComponent.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

AnchorComponent::AnchorComponent() noexcept = default;

AnchorComponent::AnchorComponent(mref<string> anchorName_) noexcept :
    InheritMeta(),
    _name(std::move(anchorName_)) {}

string_view AnchorComponent::getAnchorName() const noexcept {
    return _name;
}
