#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX/Command/__fwd.hpp>

#include "HORenderPassCreateData.hpp"
#include "HORenderPassChanges.hpp"
#include "__fwd.hpp"

namespace hg::engine::gfx {
    /**
     * Forward Declaration
     */
    class Device;
}

namespace hg::engine::gfx::render {
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
        Renderer() noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        virtual ~Renderer();

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
        [[nodiscard]] sptr<RenderPassState> makeRenderPassState() const;

    public:
        /**
         * Creates a HORenderPass for this Renderer based on given data
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
        virtual ptr<HORenderPass> allocate(mref<HORenderPassCreateData> data_);

        /**
         * Recreate a HORenderPass for this Renderer based on previous pass and changes
         *
         * @author Julius
         * @date 02.10.2022
         *
         * @details Will reuse existing RenderPass instance and cascade resource reallocation
         *  through all operators of this renderer and it's pipeline
         *
         * @param renderPass_ The RenderPass to mutate.
         * @param changes_ The changes to apply to the existing instance.
         *
         * @returns An owning pointer to the modified RenderPass when succeeded, otherwise nullptr.
         */
        virtual ptr<HORenderPass> reallocate(mref<ptr<HORenderPass>> renderPass_, mref<HORenderPassChanges> changes_);

        /**
         * Frees the given HORenderPass if allocated by this Renderer
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
        virtual bool free(mref<ptr<HORenderPass>> renderPass_);

    private:
        void invokeBatched(const non_owning_rptr<HORenderPass> renderPass_, mref<CommandBatch> batch_) const;

    public:
        virtual const non_owning_rptr<HORenderPass> invoke(const non_owning_rptr<HORenderPass> renderPass_);

        virtual const non_owning_rptr<HORenderPass> invoke(
            const non_owning_rptr<HORenderPass> renderPass_,
            cref<CommandBatch> batchLayout_
        );

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
        ptr<RenderPipeline_Deprecated> _pipeline;

    public:
        [[nodiscard]] const non_owning_rptr<const RenderPipeline_Deprecated> pipeline() const noexcept;

    protected:
        [[nodiscard]] const non_owning_rptr<RenderPipeline_Deprecated> getOrCreatePipeline();
    };
}
