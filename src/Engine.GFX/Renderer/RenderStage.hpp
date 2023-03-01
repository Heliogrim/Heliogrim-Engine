#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "Multiplexer.hpp"
#include "RenderStageDependency.hpp"
#include "__fwd.hpp"

namespace hg::engine::gfx {
    /**
     * Forward Declaration
     */
    class Device;
}

namespace hg::engine::gfx::render {
    class __declspec(novtable) RenderStage {
    public:
        using this_type = RenderStage;

        friend class RenderPipeline;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        RenderStage() = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        virtual ~RenderStage() = default;

    private:
        /**
         * Setup call from containing RenderPipeline
         *
         * @author Julius
         * @date 30.03.2022
         *
         * @details Should be invoked from containing RenderPipeline and will propagate to setup function.
         *
         * @param pipeline_ The Pipeline invoking/containing this stage.
         */
        void pipelineSetup(const non_owning_rptr<const RenderPipeline> pipeline_);

        /**
         * Destroy call from containing RenderPipeline
         *
         * @author Julius
         * @date 30.03.2022
         *
         * @details Should be invoked from containing RenderPipeline and will propagate to destroy function.
         *
         * @param pipeline_ The Pipeline invoking/containing this stage.
         */
        void pipelineDestroy(const non_owning_rptr<const RenderPipeline> pipeline_);

    protected:
        /**
         * Custom setup callback
         *
         * @author Julius
         * @date 30.03.2022
         *
         * @details Will be invoked by propagation through pipelineSetup call.
         *
         * @param device_ The device used for this render stage
         */
        virtual void setup(cref<sptr<Device>> device_);

        /**
         * Custom destroy callback
         *
         * @author Julius
         * @date 30.03.2022
         *
         * @details Will be invoked by propagation through pipelineDestroy call.
         */
        virtual void destroy();

    public:
        virtual bool allocate(const ptr<HORenderPass> renderPass_);

        virtual void free(const ptr<HORenderPass> renderPass_);

    private:
        /**
         * The stage dependencies of this RenderStage.
         *
         * @details These will effect ordering and validation of the containing RenderPipeline.
         */
        Vector<RenderStageDependency> _dependencies;

    public:
        [[nodiscard]] cref<Vector<RenderStageDependency>> dependencies() const noexcept;

    protected:
        [[nodiscard]] ref<Vector<RenderStageDependency>> dependencies() noexcept;

    public:
        bool pushDependency(cref<RenderStageDependency> dependency_);

    private:
        Multiplexer _multiplexer;

    protected:
        [[nodiscard]] ref<Multiplexer> multiplexer() noexcept;

    public:
        [[nodiscard]] cref<Multiplexer> multiplexer() const noexcept;
    };
}
