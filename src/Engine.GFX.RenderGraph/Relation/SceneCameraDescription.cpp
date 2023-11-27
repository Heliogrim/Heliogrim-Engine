#include "SceneCameraDescription.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SceneCameraDescription::SceneCameraDescription() noexcept :
    InheritMeta() {}

SceneCameraDescription::~SceneCameraDescription() = default;

bool SceneCameraDescription::isValueCompatible(const non_owning_rptr<const Description> other_) const noexcept {
    return true;
}
