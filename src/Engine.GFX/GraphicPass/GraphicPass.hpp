#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Scene/RenderGraph.hpp>

#include "GraphicPassMask.hpp"
#include "GraphicPassPipeline.hpp"
#include "GraphicPassResult.hpp"
#include "GraphicPassModelProcessor.hpp"
#include "../Swapchain/Swapchain.hpp"
#include "../Renderer/RenderInvocation.hpp"
#include "../Renderer/RenderContext.hpp"

namespace ember::engine::gfx {

    class GraphicPass {
    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @param device_ The device this pass is related to.
         * @param mask_ The mask.
         */
        GraphicPass(cref<sptr<Device>> device_, cref<GraphicPassMask> mask_);

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

    public:
        /**
         * Allocated required resources for this graphic pass and forwards to the graphic pass pipeline
         *
         * @author Julius
         * @date 04.03.2022
         *
         * @details Using virtual call allows overriding the allocation function. It is mandatory to forward the allocation call to the pipeline.
         *
         * @param invocation_ The RenderInvocation which is required for the allocation
         * @param state_ The RenderInvocationState where to store the allocated data.
         */
        virtual void allocateWith(_In_ const ptr<const RenderInvocation> invocation_,
            _Inout_ const ptr<RenderInvocationState> state_);

        /**
         * Frees the allocated resources by this graphic pass from the given state and forwards to the graphic pass pipeline
         *
         * @author Julius
         * @date 04.03.2022
         *
         * @details Using virtual call allowed overriding the free function. It is mandatory to forward the call to the pipeline.
         *
         * @param invocation_ The RenderInvocation which was used for the allocation.
         * @param state_ The RenderInvocationState where to free the resources from.
         */
        virtual void freeWith(_In_ const ptr<const RenderInvocation> invocation_,
            _Inout_ const ptr<RenderInvocationState> state_);

    public:
        /**
         * Processes the given graph_ with the state_ and populates an instruction set
         *
         * @author Julius
         * @date 10.02.2021
         *
         * @param  graph_ The graph which should be processed.
         * @param  ctx_ The RenderContext containing the resources, scene, camera and target data.
         * @param  batch_ The batch.
         */
        virtual void process(_In_ const ptr<scene::RenderGraph> graph_, _In_ const ptr<const RenderContext> ctx_,
            _Inout_ ref<CommandBatch> batch_);

    protected:
        sptr<Device> _device;

    public:
        /**
         * Returns the device this graphics pass is related to
         *
         * @author Julius
         * @date 27.11.2021
         *
         * @returns A pointer to the related device.
         */
        [[nodiscard]] const sptr<Device> device() const noexcept;

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
         * GraphicPassModelProcessor
         */

    public:
        /**
         * Creates a GraphicPassModelProcessor
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A ModelPassProcessor.
         */
        [[nodiscard]] virtual ptr<GraphicPassModelProcessor> processor() noexcept = 0;

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
