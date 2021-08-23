#include "PolygonMesh.hpp"

using namespace ember::engine::gfx;
using namespace ember;

cref<IndexBuffer> PolygonMesh::indices() const noexcept {
    return _indices;
}

ref<IndexBuffer> PolygonMesh::indices() noexcept {
    return _indices;
}

cref<VertexBuffer> PolygonMesh::vertices() const noexcept {
    return _vertices;
}

ref<VertexBuffer> PolygonMesh::vertices() noexcept {
    return _vertices;
}
