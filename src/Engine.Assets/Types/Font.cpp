#include "Font.hpp"

using namespace hg::engine::assets;
using namespace hg;

Font::Font() :
	InheritMeta(clone(invalid_asset_guid), Font::typeId) {}

Font::Font(mref<asset_guid> guid_) :
	InheritMeta(std::move(guid_), Font::typeId) {}

Font::Font(mref<asset_guid> guid_, mref<Vector<fs::Url>> sources_) :
	InheritMeta(std::move(guid_), Font::typeId),
	_sources(std::move(sources_)) {}

cref<Vector<fs::Url>> Font::sources() const noexcept {
	return _sources;
}
