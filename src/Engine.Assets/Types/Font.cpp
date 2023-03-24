#include "Font.hpp"

using namespace hg::engine::assets;
using namespace hg;

Font::Font(cref<asset_guid> guid_) :
    Asset(guid_, Font::typeId) {}

Font::Font(cref<asset_guid> guid_, mref<Vector<fs::Url>> sources_) :
    Asset(guid_, Font::typeId),
    _sources(_STD move(sources_)) {}

cref<Vector<fs::Url>> Font::sources() const noexcept {
    return _sources;
}
