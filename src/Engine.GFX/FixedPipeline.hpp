#pragma once
#include "FixedPipelineInput.hpp"
#include "GraphicPipeline.hpp"
#include "PrimitiveTopology.hpp"
#include "Pipeline/FragmentStage.hpp"
#include "Pipeline/GeometryStage.hpp"
#include "Pipeline/RasterizationStage.hpp"
#include "Pipeline/TessellationStage.hpp"
#include "Pipeline/VertexStage.hpp"

namespace ember::engine::gfx {

    class FixedPipeline :
        public GraphicPipeline {
    protected:

        /**
         * Constructor
         *
         * @author Julius
         * @date 22.01.2021
         */
        FixedPipeline() = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 22.01.2021
         */
        ~FixedPipeline() noexcept override = default;

        /**
         * Setups this 
         *
         * @author Julius
         * @date 02.12.2020
         */
        virtual void setup() = 0;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 02.12.2020
         */
        virtual void destroy() noexcept = 0;

    protected:
        /**
         * Inputs
         */
        Vector<FixedPipelineInput> _inputs;

    public:
        /**
         * Gets the inputs
         *
         * @author Julius
         * @date 01.02.2021
         *
         * @returns A cref&lt;vector&lt;FixedPipelineInput&gt;&gt;
         */
        [[nodiscard]] cref<Vector<FixedPipelineInput>> inputs() const noexcept;

        /**
         * Gets the inputs
         *
         * @author Julius
         * @date 01.02.2021
         *
         * @returns A ref&lt;vector&lt;FixedPipelineInput&gt;&gt;
         */
        [[nodiscard]] ref<Vector<FixedPipelineInput>> inputs() noexcept;

    protected:
        /**
         * Assembly
         */
        PrimitiveTopology _topology;

    public:
        /**
         * Gets the topology
         *
         * @author Julius
         * @date 18.12.2020
         *
         * @returns A reference to a PrimitiveTopology.
         */
        [[nodiscard]] PrimitiveTopology& topology() noexcept;

    protected:
        /**
         * Vertex Stage
         */
        pipeline::VertexStage _vertexStage;

    public:
        /**
         * Vertex stage
         *
         * @author Julius
         * @date 30.11.2020
         *
         * @returns A reference to a pipeline::VertexStage.
         */
        [[nodiscard]] pipeline::VertexStage& vertexStage() noexcept;

    protected:
        /**
         * Tessellation Stage
         */
        pipeline::TessellationStage _tessellationStage;

    public:
        /**
         * Tessellation stage
         *
         * @author Julius
         * @date 30.11.2020
         *
         * @returns A reference to a pipeline::TessellationStage.
         */
        [[nodiscard]] pipeline::TessellationStage& tessellationStage() noexcept;

    protected:
        /**
         * Geometry Stage
         */
        pipeline::GeometryStage _geometryStage;

    public:
        /**
         * Geometry stage
         *
         * @author Julius
         * @date 30.11.2020
         *
         * @returns A reference to a pipeline::GeometryStage.
         */
        [[nodiscard]] pipeline::GeometryStage& geometryStage() noexcept;

    protected:
        /**
         * Rasterization Stage
         */
        pipeline::RasterizationStage _rasterizationStage;

    public:
        /**
         * Rasterization stage
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns A reference to a pipeline::RasterizationStage.
         */
        [[nodiscard]] pipeline::RasterizationStage& rasterizationStage() noexcept;

    protected:
        /**
         * Fragment Stage
         */
        pipeline::FragmentStage _fragmentStage;

    public:
        /**
         * Fragment stage
         *
         * @author Julius
         * @date 30.11.2020
         *
         * @returns A reference to a pipeline::FragmentStage.
         */
        [[nodiscard]] pipeline::FragmentStage& fragmentStage() noexcept;
    };
}
