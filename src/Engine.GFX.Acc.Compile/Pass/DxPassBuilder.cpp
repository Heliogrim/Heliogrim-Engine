#include "DxPassBuilder.hpp"

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc/AccelerationPass.hpp>

using namespace hg::engine::gfx::acc;
using namespace hg;

DxPassBuilder::DxPassBuilder() = default;

DxPassBuilder::~DxPassBuilder() = default;

smr<AccelerationPass> DxPassBuilder::build(mref<smr<AccelerationEffect>> effect_) const noexcept {
    return nullptr;
}
