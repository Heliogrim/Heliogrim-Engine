#include "MaterialSubpassStage.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

void MaterialSubpassStage::operator()(nmpt<State> state_) const {
    IM_DEBUG_LOGF("-> MaterialSubpassStage `{:x}`", reinterpret_cast<ptrdiff_t>(this));
}

/*
smr<const engine::gfx::acc::AccelerationPass> MaterialSubpassStage::getAccelByMaterial(
    mref<smr<MaterialResource>> material_,
    mref<smr<acc::SpecificationStorage>> spec_
) noexcept {
    return nullptr;
}
*/
