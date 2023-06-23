#include "VkModuleBuilder.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

constexpr static const char* dummy_shader_code = "\
#version 430 core\n\
\n\
void main() {\n\
}\n\
";

VkModuleBuilder::~VkModuleBuilder() noexcept = default;

ModuleSource VkModuleBuilder::build(
    cref<smr<AccelerationPass>> targetPass_,
    cref<ScopedTokenStorage> scopedTokens_,
    cref<smr<AccelerationStageDerivat>> stage_
) const {

    ModuleSource ms {};

    switch (stage_->getFlagBits()) {
        case AccelerationStageFlagBits::eVertex: {
            ms.targetStage = ModuleTargetStage::eVertex;
            break;
        }
        case AccelerationStageFlagBits::eFragment: {
            ms.targetStage = ModuleTargetStage::eFragment;
            break;
        }
        default: {
            ms.targetStage = ModuleTargetStage::eUndefined;
            break;
        }
    }

    ms.sourceCode.push_back(dummy_shader_code);

    return ms;
}
