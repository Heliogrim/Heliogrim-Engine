#include "LandscapeGeometry.hpp"

using namespace ember::engine::assets;
using namespace ember;

LandscapeGeometry::LandscapeGeometry(cref<asset_guid> guid_) :
    Data(guid_, type_id) {}

LandscapeGeometry::LandscapeGeometry(cref<asset_guid> guid_, mref<Vector<Url>> sources_) :
    Data(guid_, type_id),
    _sources(_STD move(sources_)) {}
