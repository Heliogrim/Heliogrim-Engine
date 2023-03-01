#pragma once
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Transform.hpp>

#include "../Buffer/Buffer.hpp"

namespace hg::engine::gfx {
    struct DistinctBind { };

    struct DistinctGeometry {
        math::Transform transform;

        u32 instanceCount;
        u32 instanceOffset;

        u32 indexCount;
        u32 indexOffset;
    };

    struct SharedBind {
        Buffer uniform;
    };

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

        [[nodiscard]] ref<SharedGeometry> geometry() noexcept;

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

        [[nodiscard]] ref<SharedBind> bind() noexcept;

    private:
        /**
         * Execution Rows
         */
        Vector<ExecutionRow> _executions;

    public:
        /**
         * Gets the executions
         *
         * @author Julius
         * @date 10.02.2021
         *
         * @returns A cref&lt;vector&lt;ExecutionRow&gt;&gt;
         */
        [[nodiscard]] cref<Vector<ExecutionRow>> executions() const noexcept;

        [[nodiscard]] ref<Vector<ExecutionRow>> executions() noexcept;
    };
}
