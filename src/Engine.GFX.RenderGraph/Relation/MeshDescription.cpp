#include "MeshDescription.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

MeshDescription::MeshDescription() noexcept :
    InheritMeta() {}

MeshDescription::~MeshDescription() = default;

bool MeshDescription::isValueCompatible(const non_owning_rptr<const Description> other_) const noexcept {

    const auto& other = static_cast<cref<MeshDescription>>(*other_);

    // TODO: _layout == other._layout;
    return true;
}
