#pragma once

#include "RenderPassCreateData.hpp"
#include "RenderPassState.hpp"

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class RenderPass;
    class RenderPipeline;
}

namespace ember::engine::gfx {

    class __declspec(novtable) Renderer {
    public:
        using this_type = Renderer;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        Renderer() noexcept = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        virtual ~Renderer() = default;

    public:
        /**
         * Setup Renderer
         *
         * @author Julius
         * @date 30.03.2022
         */
        virtual void setup(cref<sptr<Device>> device_);

        /**
         * Destroy Renderer
         *
         * @author Julius
         * @date 30.03.2022
         */
        virtual void destroy() = 0;

    protected:
        [[nodiscard]] sptr<RenderPassState> makeRenderPass() const;

    public:
        /**
         * Creates a RenderPass for this Renderer based on given data
         *
         * @author Julius
         * @date 30.03.2022
         *
         * @details Will allocate a RenderPass instance and cascade resource allocation
         *  through all operators of this renderer and it's pipeline
         *
         * @param data_ The data used to created the RenderPass
         *
         * @returns A owning pointer to the created RenderPass.
         */
        virtual ptr<RenderPass> allocate(mref<RenderPassCreateData> data_);

        /**
         * Frees the given RenderPass if allocated by this Renderer
         *
         * @author Julius
         * @date 30.03.2022
         *
         * @details Will cascade freeing of resources through all operators of this renderer
         *  and delete the allocated RenderPass as long as it was allocated by this Renderer instance.
         *
         * @param renderPass_ The RenderPass to release.
         *
         * @returns True if succeeded, otherwise false.
         */
        virtual bool free(mref<ptr<RenderPass>> renderPass_);

    private:
        void invokeBatched(const non_owning_rptr<RenderPass> invocation_, mref<CommandBatch> batch_) const;

    public:
        virtual const non_owning_rptr<RenderPass> invoke(const non_owning_rptr<RenderPass> renderPass_);

        virtual const non_owning_rptr<RenderPass> invoke(const non_owning_rptr<RenderPass> renderPass_,
            cref<CommandBatch> batchLayout_);

    protected:
        /**
         * Associated api/graphic device
         */
        sptr<Device> _device;

    public:
        [[nodiscard]] cref<sptr<Device>> device() const noexcept;

    private:
        /**
         * Internal Render Pipeline
         */
        ptr<RenderPipeline> _pipeline;

    public:
        [[nodiscard]] const non_owning_rptr<const RenderPipeline> pipeline() const noexcept;

    protected:
        [[nodiscard]] const non_owning_rptr<RenderPipeline> getOrCreatePipeline();
    };
}
