#include "Font.hpp"

using namespace ember::engine::assets;
using namespace ember;

Font::Font(cref<asset_guid> guid_) :
    Asset(guid_, Font::typeId) {}

Font::Font(cref<asset_guid> guid_, mref<Vector<Url>> sources_) :
    Asset(guid_, Font::typeId),
    _sources(_STD move(sources_)) {}

cref<Vector<Url>> Font::sources() const noexcept {
    return _sources;
}
