#include "VkModuleBuilder.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

VkModuleBuilder::~VkModuleBuilder() noexcept = default;

ModuleSource VkModuleBuilder::build(
    cref<smr<AccelerationPass>> targetPass_,
    cref<ScopedTokenStorage> scopedTokens_,
    cref<smr<AccelerationStageDerivat>> stage_
) const {
    return {};
}
