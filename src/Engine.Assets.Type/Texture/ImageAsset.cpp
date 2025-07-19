#include "ImageAsset.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::assets;
using namespace hg;

ImageAsset::ImageAsset() :
	InheritMeta(clone(invalid_asset_guid), ImageAsset::typeId, StringView {}, AssetReferenceUrl {}, AssetUrl {}),
	_sources() {}

ImageAsset::ImageAsset(
	mref<AssetGuid> guid_,
	mref<StringView> name_,
	mref<AssetReferenceUrl> storageUrl_,
	mref<AssetUrl> vfsUrl_,
	mref<Vector<fs::Url>> sources_
) :
	InheritMeta(std::move(guid_), ImageAsset::typeId, ::hg::move(name_), ::hg::move(storageUrl_), ::hg::move(vfsUrl_)),
	_sources(std::move(sources_)) {}

cref<Vector<fs::Url>> ImageAsset::sources() const noexcept {
	return _sources;
}

void ImageAsset::addSource(cref<fs::Url> sourceUrl_) {
	_sources.push_back(sourceUrl_);
}

void ImageAsset::removeSource(cref<fs::Url> sourceUrl_) {
	const auto where = std::ranges::remove(
		_sources.begin(),
		_sources.end(),
		sourceUrl_.path(),
		[](cref<fs::Url> url_) {
			// TODO:
			return url_.path();
		}
	);
	_sources.erase(where.begin(), where.end());
}
