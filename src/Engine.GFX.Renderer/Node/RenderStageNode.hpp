#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

#include "../__fwd.hpp"

namespace hg::engine::gfx {
    /**
     * Forward Declaration
     */
    class Device;
    class SceneNodeModel;
}

namespace hg::engine::gfx::render {
    class __declspec(novtable) RenderStageNode {
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
        virtual void setup(cref<sptr<Device>> device_);

        virtual void destroy();

    public:
        virtual bool allocate(const ptr<HORenderPass> renderPass_);

        virtual bool free(const ptr<HORenderPass> renderPass_);

    public:
        [[nodiscard]] virtual Vector<RenderDataToken> requiredToken() noexcept = 0;

        [[nodiscard]] virtual Vector<RenderDataToken> optionalToken() noexcept = 0;

    public:
        /**
         * Get the processable model types of this RenderStageNode
         *
         * @details Will be requested by Multiplexer to build execution mesh and filter invocations.
         *  // TODO: Checkout whether we can make a template derived type of RenderStageNode providing method overrides with optimized `variant<...Types>`
         *
         * @returns A list of type_ids describing the SceneNodeModel types
         */
        [[nodiscard]] virtual const non_owning_rptr<const Vector<type_id>> modelTypes() const noexcept;

    public:
        virtual void before(
            const non_owning_rptr<HORenderPass> renderPass_,
            const non_owning_rptr<RenderStagePass> stagePass_
        ) const;

        virtual void invoke(
            const non_owning_rptr<HORenderPass> renderPass_,
            const non_owning_rptr<RenderStagePass> stagePass_,
            const non_owning_rptr<SceneNodeModel> model_
        ) const;

        virtual void after(
            const non_owning_rptr<HORenderPass> renderPass_,
            const non_owning_rptr<RenderStagePass> stagePass_
        ) const;
    };
}
