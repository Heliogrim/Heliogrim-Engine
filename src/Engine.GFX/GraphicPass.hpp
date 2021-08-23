#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Scene/Graph/SceneGraph.hpp>

#include "GraphicPassMask.hpp"
#include "GraphicPassPipeline.hpp"
#include "GraphicPassResult.hpp"
#include "ModelPassProcessor.hpp"

namespace ember::engine::gfx {
    class GraphicPass {
    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @param  mask_ The mask.
         */
        GraphicPass(cref<GraphicPassMask> mask_);

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 17.01.2021
         */
        virtual ~GraphicPass() = default;

        /**
         * Setups this 
         *
         * @author Julius
         * @date 17.01.2021
         */
        virtual void setup() = 0;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 17.01.2021
         */
        virtual void destroy() = 0;

        /**
         * Processes the given graph and returns instruction sets
         *
         * @author Julius
         * @date 10.02.2021
         *
         * @param  graph_ The SceneGraph to traverse.
         * @param  batch_ The batch.
         */
        [[nodiscard]] virtual void process(cref<scene::SceneGraph> graph_, IN OUT ref<CommandBatch> batch_);

    private:
        /**
         * GraphicPassMask
         */
        GraphicPassMask _mask;

    public:
        /**
         * Gets the mask
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A RenderPassMask.
         */
        [[nodiscard]] GraphicPassMask mask() const noexcept;

        /**
         * Gets the mask
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A RenderPassMask.
         */
        [[nodiscard]] GraphicPassMask mask() noexcept;

    private:
        /**
         * ModelPassProcessor
         */

    public:
        /**
         * Creates a ModelPassProcessor
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A ModelPassProcessor.
         */
        [[nodiscard]] virtual ptr<ModelPassProcessor> processor() noexcept = 0;

    protected:
        /**
         * GraphicPassPipeline
         */
        GraphicPassPipeline _pipeline;

    public:
        /**
         * Gets the pipeline
         *
         * @author Julius
         * @date 17.01.2021
         *
         * @returns A cref&lt;RenderPassPipeline&gt;
         */
        [[nodiscard]] cref<GraphicPassPipeline> pipeline() const noexcept;
    };
}
