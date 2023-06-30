#include "VkPassBuilder.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc/Pass/VkAccelerationGraphicsPass.hpp>

#include "../Spec/SpecificationStorage.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

VkPassBuilder::VkPassBuilder() = default;

VkPassBuilder::~VkPassBuilder() = default;

smr<AccelerationPass> VkPassBuilder::build(
    mref<smr<AccelerationEffect>> effect_,
    cref<class SpecificationStorage> specifications_
) const noexcept {
    assert(specifications_.getGraphicsSpec().renderPass);
    return make_smr<VkAccelerationGraphicsPass>(
        VkAccelerationGraphicsPass::create<VkAccelerationGraphicsPass>(_STD move(effect_))
    );
}
