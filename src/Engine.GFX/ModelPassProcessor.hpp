#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Scene/Node/SceneNode.hpp>

#include "ProcessedModelBatch.hpp"

namespace ember::engine::gfx {

    /**
     * Forward Declaration
     */
    class GraphicPass;

    class ModelPassProcessor {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 22.03.2021
         *
         * @param  graphicPass_ The graphic pass.
         */
        ModelPassProcessor(ptr<const GraphicPass> graphicPass_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 22.03.2021
         */
        virtual ~ModelPassProcessor() = default;

    public:
        /**
         * Function call operator
         *
         * @author Julius
         * @date 09.01.2021
         *
         * @param batchIdx_ The current batch index.
         * @param  node_ The node.
         *
         * @returns The result of the operation.
         */
        [[nodiscard]] virtual bool operator()(u32 batchIdx_, cref<scene::SceneNode> node_) noexcept;

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
         * @returns A cref&lt;vector&lt;ProcessedModelBatch&gt;&gt;
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
