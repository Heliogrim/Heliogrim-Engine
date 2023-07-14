#include "MaterialSubpassStage.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

void MaterialSubpassStage::operator()(nmpt<State> state_) const {}

smr<const engine::gfx::acc::AccelerationPass> MaterialSubpassStage::getAccelByMaterial(
    mref<smr<MaterialResource>> material_,
    mref<smr<acc::SpecificationStorage>> spec_
) noexcept {
    return nullptr;
}
