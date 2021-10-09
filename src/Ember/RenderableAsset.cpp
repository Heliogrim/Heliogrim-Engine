#include "RenderableAsset.hpp"

using namespace ember;

RenderableAsset::RenderableAsset(cref<asset_guid> guid_, cref<asset_type_id> typeId_, ptr<void> internal_) noexcept :
    Asset(guid_, typeId_, internal_) {}

RenderableAsset::~RenderableAsset() noexcept = default;
