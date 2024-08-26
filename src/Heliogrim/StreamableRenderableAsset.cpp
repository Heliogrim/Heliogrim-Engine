#include "StreamableRenderableAsset.hpp"

using namespace hg;

StreamableRenderableAsset::StreamableRenderableAsset(
	mref<asset_guid> guid_,
	cref<asset_type_id> typeId_,
	ref<::hg::engine::assets::Asset> internal_
) noexcept :
	RenderableAsset(std::move(guid_), typeId_, internal_) {}

StreamableRenderableAsset::~StreamableRenderableAsset() noexcept = default;
