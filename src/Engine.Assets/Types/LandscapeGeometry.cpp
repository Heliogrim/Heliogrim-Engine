#include "LandscapeGeometry.hpp"

using namespace hg::engine::assets;
using namespace hg;

LandscapeGeometry::LandscapeGeometry(cref<asset_guid> guid_) :
    InheritMeta(guid_, typeId) {}

LandscapeGeometry::LandscapeGeometry(cref<asset_guid> guid_, mref<Vector<fs::Url>> sources_) :
    InheritMeta(guid_, typeId),
    _sources(std::move(sources_)) {}
