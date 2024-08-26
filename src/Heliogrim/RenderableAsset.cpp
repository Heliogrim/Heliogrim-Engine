#include "RenderableAsset.hpp"

using namespace hg;

RenderableAsset::RenderableAsset(
	mref<asset_guid> guid_,
	cref<asset_type_id> typeId_,
	ref<::hg::engine::assets::Asset> internal_
) noexcept :
	Asset(std::move(guid_), typeId_, internal_) {}

RenderableAsset::~RenderableAsset() noexcept = default;
