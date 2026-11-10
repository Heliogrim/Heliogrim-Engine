#pragma once

#include "Mesh.hpp"
#include "../Buffer/SparseBufferView.hpp"

namespace hg::engine::gfx {
    class PolygonMesh :
        public Mesh {
    public:
        using vertex_buffer_type = uptr<SparseBufferView>;
        using index_buffer_type = uptr<SparseBufferView>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 19.02.2021
         */
        constexpr PolygonMesh() noexcept = default;

        PolygonMesh(mref<vertex_buffer_type> vertices_, mref<index_buffer_type> indices_) noexcept;

        ~PolygonMesh();

    protected:
        /**
         * Vertex Buffer
         */
        vertex_buffer_type _vertices;

    public:
        /**
         * Gets the vertices
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A const reference to the vertex buffer
         */
        [[nodiscard]] cref<vertex_buffer_type> vertices() const noexcept;

        /**
         * Gets the vertices
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A ref&lt;VertexBuffer&gt;
         */
        [[nodiscard]] ref<vertex_buffer_type> vertices() noexcept;

    protected:
        /**
         * Index Buffer
         */
        index_buffer_type _indices;

    public:
        /**
         * Gets the indices
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A const reference to the index buffer
         */
        [[nodiscard]] cref<index_buffer_type> indices() const noexcept;

        /**
         * Gets the indices
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A ref&lt;IndexBuffer&gt;
         */
        [[nodiscard]] ref<index_buffer_type> indices() noexcept;
    };
}
