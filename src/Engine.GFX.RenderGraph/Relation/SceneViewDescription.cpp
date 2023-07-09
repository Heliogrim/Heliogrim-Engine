#include "SceneViewDescription.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SceneViewDescription::SceneViewDescription() noexcept :
    Description() {}

SceneViewDescription::~SceneViewDescription() = default;

bool SceneViewDescription::isValueCompatible(const non_owning_rptr<const Description> other_) const noexcept {
    return true;
}
