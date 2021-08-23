#pragma once
#include <Engine.Common/Collection/List.hpp>
#include <Engine.Common/Math/Transformation.hpp>

#include "Buffer/Buffer.hpp"

namespace ember::engine::gfx {

    struct DistinctBind { };

    struct DistinctGeometry {
        math::Transformation transformation;

        u32 instanceCount;
        u32 instanceOffset;

        u32 indexCount;
        u32 indexOffset;
    };

    struct SharedBind { };

    struct SharedGeometry {
        VertexBuffer vertices;

        Buffer instances;

        IndexBuffer indices;
    };

    struct ExecutionRow {
        DistinctBind bind;
        DistinctGeometry geometry;
    };

    class ProcessedModelBatch {
    public:

        /**
         * Check batch is empty
         *
         * @author Julius
         * @date 09.02.2021
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool empty() const noexcept;

    private:
        /**
         * Shared Geometry
         */
        SharedGeometry _geometry;

    public:
        /**
         * Gets the geometry
         *
         * @author Julius
         * @date 09.02.2021
         *
         * @returns A cref&lt;SharedGeometry&gt;
         */
        [[nodiscard]] cref<SharedGeometry> geometry() const noexcept;

    private:
        /**
         * Shared Bind
         */
        SharedBind _bind;

    public:
        /**
         * Gets the bind
         *
         * @author Julius
         * @date 09.02.2021
         *
         * @returns A cref&lt;SharedBind&gt;
         */
        [[nodiscard]] cref<SharedBind> bind() const noexcept;

    private:
        /**
         * Execution Rows
         */
        vector<ExecutionRow> _executions;

    public:
        /**
         * Gets the executions
         *
         * @author Julius
         * @date 10.02.2021
         *
         * @returns A cref&lt;vector&lt;ExecutionRow&gt;&gt;
         */
        [[nodiscard]] cref<vector<ExecutionRow>> executions() const noexcept;
    };
}
