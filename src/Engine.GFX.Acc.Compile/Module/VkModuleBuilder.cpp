#include "VkModuleBuilder.hpp"

#include "VkModuleSource.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

constexpr static const char* dummy_shader_code = "\
#version 430 core\n\
\n\
void main() {\n\
}\n\
";

VkModuleBuilder::~VkModuleBuilder() noexcept = default;

uptr<ModuleSource> VkModuleBuilder::build(
    cref<smr<AccelerationPass>> targetPass_,
    cref<ScopedTokenStorage> scopedTokens_,
    cref<smr<AccelerationStageDerivat>> stage_
) const {

    auto ms = make_uptr<VkModuleSource>();

    /**/

    switch (stage_->getFlagBits()) {
        case AccelerationStageFlagBits::eVertex: {
            ms->targetStage = ModuleTargetStage::eVertex;
            break;
        }
        case AccelerationStageFlagBits::eFragment: {
            ms->targetStage = ModuleTargetStage::eFragment;
            break;
        }
        default: {
            ms->targetStage = ModuleTargetStage::eUndefined;
            break;
        }
    }

    /* Resolve tokenized binding layouts to platform specific location bindings */

    s32 majorIndex = 0i32;
    s32 minorIndex = 0i32;

    for (const auto& layout : targetPass_->getPassBindings().layouts) {

        for (const auto& element : layout.elements) {

            ms->mappedLocations.insert(
                _STD make_pair(
                    element.token,
                    VkBindingLocation {
                        majorIndex,
                        minorIndex
                    }
                )
            );

            ++minorIndex;
        }

        minorIndex = 0i32;
        ++majorIndex;
    }

    /* Transform IL shader source code to platform shader source code. */

    ms->sourceCode.push_back(dummy_shader_code);

    /**/

    return ms;
}
