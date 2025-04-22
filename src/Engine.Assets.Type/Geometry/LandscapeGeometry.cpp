#include "LandscapeGeometry.hpp"

using namespace hg::engine::assets;
using namespace hg;

LandscapeGeometry::LandscapeGeometry(mref<AssetGuid> guid_) :
    InheritMeta(std::move(guid_), typeId) {}

LandscapeGeometry::LandscapeGeometry(mref<AssetGuid> guid_, mref<Vector<fs::Url>> sources_) :
    InheritMeta(std::move(guid_), typeId),
    _sources(std::move(sources_)) {}
