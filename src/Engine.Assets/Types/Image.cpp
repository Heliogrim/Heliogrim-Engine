#include "Image.hpp"

#include <algorithm>

using namespace hg::engine::assets;
using namespace hg;

Image::Image() :
    InheritMeta(invalid_asset_guid, Image::typeId) {}

Image::Image(cref<asset_guid> guid_) :
    InheritMeta(guid_, Image::typeId) {}

Image::Image(cref<asset_guid> guid_, mref<Vector<fs::Url>> sources_) :
    InheritMeta(guid_, Image::typeId),
    _sources(_STD move(sources_)) {}

cref<Vector<fs::Url>> Image::sources() const noexcept {
    return _sources;
}

void Image::addSource(cref<fs::Url> sourceUrl_) {
    _sources.push_back(sourceUrl_);
}

void Image::removeSource(cref<fs::Url> sourceUrl_) {
    const auto where = _STD ranges::remove(
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
