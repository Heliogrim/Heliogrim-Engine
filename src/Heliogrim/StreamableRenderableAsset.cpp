#include "StreamableRenderableAsset.hpp"

using namespace hg;

StreamableRenderableAsset::StreamableRenderableAsset(
    cref<asset_guid> guid_,
    cref<asset_type_id> typeId_,
    ptr<void> internal_
) noexcept :
    RenderableAsset(guid_, typeId_, internal_) {}

StreamableRenderableAsset::~StreamableRenderableAsset() noexcept = default;
