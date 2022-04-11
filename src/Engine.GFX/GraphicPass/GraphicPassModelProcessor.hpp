#pragma once

#if FALSE

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Scene/RenderGraph.hpp>

#include "../ModelBatch.hpp"
#include "ProcessedModelBatch.hpp"
#include "../Renderer/RenderContext.hpp"

namespace ember::engine::gfx {

    /**
     * Forward Declaration
     */
    class GraphicPass;

    class GraphicPassModelProcessor {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 22.03.2021
         *
         * @param  graphicPass_ The graphic pass.
         */
        GraphicPassModelProcessor(ptr<const GraphicPass> graphicPass_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 22.03.2021
         */
        virtual ~GraphicPassModelProcessor() = default;

    public:
        /**
         * Function call operator
         *
         * @author Julius
         * @date 09.01.2021
         *
         * @param batchIdx_ The current batch index.
         * @param node_ The node.
         *
         * @returns The result of the operation.
         */
        [[nodiscard]] virtual bool operator()(u32 batchIdx_, const ptr<scene::RenderGraph::node_type> node_) noexcept;

        /**
         * Processes a single model data batch and stores resulting data internally
         *
         * @author Julius
         * @date 07.01.2022
         *
         * @param model_ The model data to process.
         */
        virtual void process(mref<ModelBatch> model_) noexcept;

        /**
         * Post processing of the consumed data
         *
         * @author Julius
         * @date 11.03.2022
         *
         * @param  ctx_ The RenderContext containing the resources, scene, camera and target data.
         */
        virtual void postProcess(const ptr<const RenderContext> ctx_);

    protected:
        /**
         * Process Model Batches
         */
        Vector<ProcessedModelBatch> _consumed;

    public:
        /**
         * Gets the previous consumed ModelBatches as list of Models
         *
         * @author Julius
         * @date 22.03.2021
         *
         * @returns A collection of ProcessedModelBatch
         */
        [[nodiscard]] cref<Vector<ProcessedModelBatch>> models() const noexcept;

    protected:
        /**
         * Render Pass
         */
        ptr<const GraphicPass> _graphicPass = nullptr;

    public:
        /**
         * Get origin GraphicPass
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A ptr&lt;RenderPass&gt;
         */
        [[nodiscard]] ptr<const GraphicPass> graphicPass() const noexcept;

        /**
         * Get origin GraphicPass
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A ptr&lt;RenderPass&gt;
         */
        [[nodiscard]] ptr<const GraphicPass> graphicPass() noexcept;
    };
}

#endif
