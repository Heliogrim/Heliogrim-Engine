#include "ModelDescription.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

ModelDescription::ModelDescription() noexcept :
    InheritMeta() {}

ModelDescription::~ModelDescription() = default;

bool ModelDescription::isValueCompatible(const non_owning_rptr<const Description> other_) const noexcept {
    return true;
}
