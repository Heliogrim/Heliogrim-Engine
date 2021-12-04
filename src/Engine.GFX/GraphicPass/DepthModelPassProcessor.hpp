#pragma once
#include "../ModelPassProcessor.hpp"

namespace ember::engine::gfx {

    class DepthModelPassProcessor :
        public ModelPassProcessor {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 22.03.2021
         *
         * @param  graphicPass_ The graphic pass.
         */
        DepthModelPassProcessor(ptr<const GraphicPass> graphicPass_);

        /**
         * Function call operator
         *
         * @author Julius
         * @date 22.03.2021
         *
         * @param batchIdx_ The current batch index.
         * @param  node_ The node.
         *
         * @returns The result of the operation.
         */
        [[nodiscard]] bool operator()(u32 batchIdx_, cref<scene::RenderGraph::node_type> node_) noexcept override;

        // Temporary test solution; Might be moved to base class and enforced to derived ones
        void reset();
    };
}
