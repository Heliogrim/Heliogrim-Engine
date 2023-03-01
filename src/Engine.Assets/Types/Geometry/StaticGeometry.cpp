#include "StaticGeometry.hpp"

using namespace hg::engine::assets;
using namespace hg;

StaticGeometry::StaticGeometry(cref<asset_guid> guid_) :
    Asset(guid_, typeId) {}

StaticGeometry::StaticGeometry(
    cref<asset_guid> guid_,
    mref<Vector<Url>> sources_,
    cref<u64> vertexCount_,
    cref<u64> indexCount_
) :
    Asset(guid_, typeId),
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

u32 StaticGeometry::getMaterialCount() const {
    // TODO:
    return 1ui32;
}
