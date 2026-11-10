#include "SceneViewDescription.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

SceneViewDescription::SceneViewDescription() noexcept :
    InheritMeta() {}

SceneViewDescription::~SceneViewDescription() = default;

bool SceneViewDescription::isValueCompatible(const non_owning_rptr<const Description> other_) const noexcept {
    return true;
}
