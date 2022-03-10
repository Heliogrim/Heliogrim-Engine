#include "StaticGeometry.hpp"

using namespace ember::engine::assets;
using namespace ember;

StaticGeometry::StaticGeometry(cref<asset_guid> guid_) :
    Data(guid_, type_id) {}

StaticGeometry::StaticGeometry(cref<asset_guid> guid_, mref<Vector<Url>> sources_) :
    Data(guid_, type_id),
    _sources(_STD move(sources_)) {}
