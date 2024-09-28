#include "Image.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::assets;
using namespace hg;

Image::Image() :
	InheritMeta(clone(invalid_asset_guid), Image::typeId) {}

Image::Image(mref<asset_guid> guid_) :
	InheritMeta(std::move(guid_), Image::typeId) {}

Image::Image(mref<asset_guid> guid_, mref<Vector<fs::Url>> sources_) :
	InheritMeta(std::move(guid_), Image::typeId),
	_sources(std::move(sources_)) {}

cref<Vector<fs::Url>> Image::sources() const noexcept {
	return _sources;
}

void Image::addSource(cref<fs::Url> sourceUrl_) {
	_sources.push_back(sourceUrl_);
}

void Image::removeSource(cref<fs::Url> sourceUrl_) {
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
