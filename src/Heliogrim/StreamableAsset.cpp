#include "StreamableAsset.hpp"

using namespace hg;

StreamableAsset::StreamableAsset(
	mref<asset_guid> guid_,
	cref<asset_type_id> typeId_,
	ref<::hg::engine::assets::Asset> internal_
) noexcept :
	Asset(
		std::move(guid_),
		typeId_,
		internal_
	) {}

StreamableAsset::~StreamableAsset() noexcept = default;
