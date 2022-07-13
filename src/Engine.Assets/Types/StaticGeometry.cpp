#include "StaticGeometry.hpp"

using namespace ember::engine::assets;
using namespace ember;

StaticGeometry::StaticGeometry(cref<asset_guid> guid_) :
    Data(guid_, typeId) {}

StaticGeometry::StaticGeometry(cref<asset_guid> guid_, mref<Vector<Url>> sources_) :
    Data(guid_, typeId),
    _sources(_STD move(sources_)) {}

u64 StaticGeometry::getVertexCount() const {
    return 1140ui64;
}

u64 StaticGeometry::getIndexCount() const {
    return 1140ui64;
}
