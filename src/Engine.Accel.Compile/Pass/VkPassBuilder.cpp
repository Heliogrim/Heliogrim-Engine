#include "VkPassBuilder.hpp"

#include <Engine.Accel.Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "../Spec/EffectSpecification.hpp"
#include "../Profile/EffectProfile.hpp"

using namespace hg::engine::accel;
using namespace hg;

VkPassBuilder::VkPassBuilder() = default;

VkPassBuilder::~VkPassBuilder() = default;

smr<AccelerationPipeline> VkPassBuilder::build(
    mref<smr<const AccelerationEffect>> effect_,
    cref<smr<const class EffectSpecification>> specifications_,
    cref<smr<const class EffectProfile>> profile_
) const noexcept {
    //assert(specifications_.getGraphicsSpec().renderPass);
    return make_smr<VkGraphicsPipeline>(
        new VkGraphicsPipeline(_STD move(effect_))
    );
}
