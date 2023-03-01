#include "LandscapeGeometry.hpp"

using namespace hg::engine::assets;
using namespace hg;

LandscapeGeometry::LandscapeGeometry(cref<asset_guid> guid_) :
    Asset(guid_, typeId) {}

LandscapeGeometry::LandscapeGeometry(cref<asset_guid> guid_, mref<Vector<Url>> sources_) :
    Asset(guid_, typeId),
    _sources(_STD move(sources_)) {}
