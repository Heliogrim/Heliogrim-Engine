#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class RenderPass;
    class SceneNodeModel;
}

namespace ember::engine::gfx {

    class RenderStageNode {
    public:
        using this_type = RenderStageNode;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        RenderStageNode() = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        virtual ~RenderStageNode() = default;

    public:
        virtual void setup();

        virtual void destroy();

    public:
        virtual bool allocate(const ptr<const RenderPass> renderPass_);

        virtual bool free(const ptr<const RenderPass> renderPass_);

    public:
        virtual void invoke(const non_owning_rptr<RenderPass> renderPass_,
            const non_owning_rptr<SceneNodeModel> model_) const;
    };
}
