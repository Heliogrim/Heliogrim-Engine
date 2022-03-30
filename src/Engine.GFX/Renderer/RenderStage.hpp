#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "Multiplexer.hpp"
#include "RenderStageDependency.hpp"

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class RenderPass;
    class RenderPipeline;
}

namespace ember::engine::gfx {

    class RenderStage {
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
         */
        virtual void setup();

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
        bool allocate(const ptr<const RenderPass> renderPass_);

        void free(const ptr<const RenderPass> renderPass_);

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

    private:
        Multiplexer _multiplexer;

    public:
        [[nodiscard]] cref<Multiplexer> multiplexer() const noexcept;
    };
}
