#include "PolygonMesh.hpp"

#include "../Buffer/SparseBufferView.hpp"

using namespace hg::engine::gfx;
using namespace hg;

PolygonMesh::PolygonMesh(mref<vertex_buffer_type> vertices_, mref<index_buffer_type> indices_) noexcept :
    _vertices(_STD move(vertices_)),
    _indices(_STD move(indices_)) {}

PolygonMesh::~PolygonMesh() = default;

cref<PolygonMesh::vertex_buffer_type> PolygonMesh::vertices() const noexcept {
    return _vertices;
}

ref<PolygonMesh::vertex_buffer_type> PolygonMesh::vertices() noexcept {
    return _vertices;
}

cref<PolygonMesh::index_buffer_type> PolygonMesh::indices() const noexcept {
    return _indices;
}

ref<PolygonMesh::index_buffer_type> PolygonMesh::indices() noexcept {
    return _indices;
}
