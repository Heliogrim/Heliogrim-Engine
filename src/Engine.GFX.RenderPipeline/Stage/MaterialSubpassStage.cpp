#include "MaterialSubpassStage.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

MaterialSubpassStage::MaterialSubpassStage() noexcept :
    SubpassStage(),
    _stagedAccelPasses() {}

MaterialSubpassStage::MaterialSubpassStage(
    mref<DenseMap<smr<const MaterialResource>, Vector<StagedAccelPass>>> stagedPasses_
) noexcept :
    SubpassStage(),
    _stagedAccelPasses(_STD move(stagedPasses_)) {}

MaterialSubpassStage::~MaterialSubpassStage() = default;

void MaterialSubpassStage::operator()(nmpt<State> state_) const {
    IM_DEBUG_LOGF("-> MaterialSubpassStage `{:x}`", reinterpret_cast<ptrdiff_t>(this));
}

/*
smr<const engine::gfx::acc::AccelerationPipeline> MaterialSubpassStage::getAccelByMaterial(
    mref<smr<MaterialResource>> material_,
    mref<smr<acc::SpecificationStorage>> spec_
) noexcept {
    return nullptr;
}
*/
