#include "LandscapeGeometry.hpp"

using namespace hg::engine::assets;
using namespace hg;

LandscapeGeometry::LandscapeGeometry(
	mref<AssetGuid> guid_,
	mref<StringView> name_,
	mref<AssetReferenceUrl> storageUrl_,
	mref<AssetUrl> vfsUrl_,
	mref<Vector<fs::Url>> sources_
) :
	InheritMeta(::hg::move(guid_), typeId, ::hg::move(name_), ::hg::move(storageUrl_), ::hg::move(vfsUrl_)),
	_sources(::hg::move(sources_)) {}
