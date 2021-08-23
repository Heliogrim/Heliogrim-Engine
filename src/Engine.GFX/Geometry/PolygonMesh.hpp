#pragma once

#include "Mesh.hpp"
#include "../Buffer/Buffer.hpp"

namespace ember::engine::gfx {
    class PolygonMesh :
        public Mesh {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 19.02.2021
         */
        PolygonMesh() = default;

    protected:
        /**
         * Vertex Buffer
         */
        VertexBuffer _vertices;

    public:

        /**
         * Gets the vertices
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A cref&lt;VertexBuffer&gt;
         */
        [[nodiscard]] cref<VertexBuffer> vertices() const noexcept;

        /**
         * Gets the vertices
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A ref&lt;VertexBuffer&gt;
         */
        [[nodiscard]] ref<VertexBuffer> vertices() noexcept;

    protected:
        /**
         * Index Buffer
         */
        IndexBuffer _indices;

    public:

        /**
         * Gets the indices
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A cref&lt;IndexBuffer&gt;
         */
        [[nodiscard]] cref<IndexBuffer> indices() const noexcept;

        /**
         * Gets the indices
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A ref&lt;IndexBuffer&gt;
         */
        [[nodiscard]] ref<IndexBuffer> indices() noexcept;
    };
}
