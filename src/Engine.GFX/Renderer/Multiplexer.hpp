#pragma once

#include "RenderStageNode.hpp"

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class RenderPass;
    class SceneNodeModel;
}

namespace ember::engine::gfx {

    class Multiplexer {
    public:
        using this_type = Multiplexer;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        Multiplexer() = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        ~Multiplexer() = default;

    public:
        void setup();

        void destroy();

    public:
        bool allocate(const ptr<const RenderPass> renderPass_);

        void free(const ptr<const RenderPass> renderPass_);

    public:
        void dispatch(const non_owning_rptr<RenderPass> renderPass_,
            const non_owning_rptr<SceneNodeModel> model_) const;

    public:
        void push(cref<sptr<RenderStageNode>> node_);

        bool pop(cref<sptr<RenderStageNode>> node_);
    };
}
