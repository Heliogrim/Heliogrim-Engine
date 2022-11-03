#include "LandscapeGeometry.hpp"

using namespace ember::engine::assets;
using namespace ember;

LandscapeGeometry::LandscapeGeometry(cref<asset_guid> guid_) :
    Asset(guid_, typeId) {}

LandscapeGeometry::LandscapeGeometry(cref<asset_guid> guid_, mref<Vector<Url>> sources_) :
    Asset(guid_, typeId),
    _sources(_STD move(sources_)) {}
