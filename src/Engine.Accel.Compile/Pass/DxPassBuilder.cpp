#include "DxPassBuilder.hpp"

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

using namespace hg::engine::accel;
using namespace hg;

DxPassBuilder::DxPassBuilder() = default;

DxPassBuilder::~DxPassBuilder() = default;

smr<AccelerationPipeline> DxPassBuilder::build(
    mref<smr<const AccelerationEffect>> effect_,
    cref<smr<const class EffectSpecification>> specifications_,
    cref<smr<const class EffectProfile>> profile_
) const noexcept {
    return nullptr;
}
