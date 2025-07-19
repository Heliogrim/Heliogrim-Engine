#include "FontAsset.hpp"

using namespace hg::engine::assets;
using namespace hg;

FontAsset::FontAsset() :
	InheritMeta(clone(invalid_asset_guid), FontAsset::typeId, StringView {}, AssetReferenceUrl {}, AssetUrl {}),
	_sources() {}

FontAsset::FontAsset(
	mref<AssetGuid> guid_,
	mref<StringView> name_,
	mref<AssetReferenceUrl> storageUrl_,
	mref<AssetUrl> vfsUrl_,
	mref<Vector<fs::Url>> sources_
) :
	InheritMeta(std::move(guid_), FontAsset::typeId, ::hg::move(name_), ::hg::move(storageUrl_), ::hg::move(vfsUrl_)),
	_sources(std::move(sources_)) {}

cref<Vector<fs::Url>> FontAsset::sources() const noexcept {
	return _sources;
}
