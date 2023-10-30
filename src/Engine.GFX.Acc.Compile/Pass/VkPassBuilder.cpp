#include "VkPassBuilder.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc/Pipeline/VkGraphicsPipeline.hpp>

#include "../Spec/EffectSpecification.hpp"
#include "../Profile/EffectProfile.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

VkPassBuilder::VkPassBuilder() = default;

VkPassBuilder::~VkPassBuilder() = default;

smr<AccelerationPipeline> VkPassBuilder::build(
    mref<smr<const AccelerationEffect>> effect_,
    cref<class EffectSpecification> specifications_,
    cref<smr<const class EffectProfile>> profile_
) const noexcept {
    //assert(specifications_.getGraphicsSpec().renderPass);
    return make_smr<VkGraphicsPipeline>(
        new VkGraphicsPipeline(_STD move(effect_))
    );
}
