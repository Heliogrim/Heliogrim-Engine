#include "RenderableAsset.hpp"

using namespace hg;

RenderableAsset::RenderableAsset(mref<asset_guid> guid_, cref<asset_type_id> typeId_, ptr<void> internal_) noexcept :
    Asset(std::move(guid_), typeId_, internal_) {}

RenderableAsset::~RenderableAsset() noexcept = default;
