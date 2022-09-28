#include "StaticGeometry.hpp"

using namespace ember::engine::assets;
using namespace ember;

StaticGeometry::StaticGeometry(cref<asset_guid> guid_) :
    Data(guid_, typeId) {}

StaticGeometry::StaticGeometry(
    cref<asset_guid> guid_,
    mref<Vector<Url>> sources_,
    cref<u64> vertexCount_,
    cref<u64> indexCount_
) :
    Data(guid_, typeId),
    _sources(_STD move(sources_)),
    _vertexCount(vertexCount_),
    _indexCount(indexCount_) {}

cref<Vector<Url>> StaticGeometry::sources() const noexcept {
    return _sources;
}

u64 StaticGeometry::getVertexCount() const {
    return _vertexCount;
}

u64 StaticGeometry::getIndexCount() const {
    return _indexCount;
}
