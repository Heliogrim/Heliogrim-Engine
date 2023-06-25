#include "VkModuleBuilder.hpp"

#include <Engine.GFX.Acc/Pass/VkAccelerationGraphicsPass.hpp>

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
    cref<class SpecificationStorage> specifications_,
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

    if (targetPass_->getClass()->isExactType<VkAccelerationGraphicsPass>() &&
        stage_->getFlagBits() == AccelerationStageFlagBits::eVertex
    ) {

        constexpr s32 invalidSet = -1i32;
        s32 location = 0i32;

        for (const auto& sib : stage_->getStageInputs()) {

            if (sib.transferType != AccelerationStageTransferType::eForward) {
                continue;
            }

            ms->mappedLocations.insert(
                _STD make_pair(
                    sib.token,
                    VkBindingLocation {
                        invalidSet,
                        location++
                    }
                )
            );

        }

    }

    /**/

    s32 dsetIndex = 0i32;
    s32 dsetLocation = 0i32;

    // Warning: This has to be invariant over all stages within one target pass
    for (const auto& layout : targetPass_->getPassBindings().layouts) {

        for (const auto& element : layout.elements) {

            ms->mappedLocations.insert(
                _STD make_pair(
                    element.token,
                    VkBindingLocation {
                        dsetIndex,
                        dsetLocation
                    }
                )
            );

            ++dsetLocation;
        }

        dsetLocation = 0i32;
        ++dsetIndex;
    }

    /* Transform IL shader source code to platform shader source code. */

    ms->sourceCode.push_back(dummy_shader_code);

    /**/

    return ms;
}
