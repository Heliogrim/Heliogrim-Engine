#include "Image.hpp"

#include <algorithm>

using namespace ember::engine::assets;
using namespace ember;

Image::Image() :
    Asset(invalid_asset_guid, Image::typeId) {}

Image::Image(cref<asset_guid> guid_) :
    Asset(guid_, Image::typeId) {}

Image::Image(cref<asset_guid> guid_, mref<Vector<Url>> sources_) :
    Asset(guid_, Image::typeId),
    _sources(_STD move(sources_)) {}

cref<Vector<Url>> Image::sources() const noexcept {
    return _sources;
}

void Image::addSource(cref<Url> sourceUrl_) {
    _sources.push_back(sourceUrl_);
}

void Image::removeSource(cref<Url> sourceUrl_) {
    const auto where = _STD ranges::remove(_sources.begin(), _sources.end(), sourceUrl_.path(), [](cref<Url> url_) {
        // TODO:
        return url_.path();
    });
    _sources.erase(where.begin(), where.end());
}
