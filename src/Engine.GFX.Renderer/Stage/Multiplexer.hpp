#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/BytellHashMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

#include "../__fwd.hpp"

namespace hg::engine::gfx {
    class Device;
    class SceneNodeModel;
}

namespace hg::engine::gfx::render {
    struct MultiplexerNodeMesh {
        using node_container_type = Vector<ptr<RenderStageNode>>;
        using mapping_type = ska::bytell_hash_map<type_id, node_container_type>;

        mapping_type mapping;
    };

    class Multiplexer {
    public:
        using this_type = Multiplexer;

        using node_container_type = Vector<ptr<RenderStageNode>>;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        Multiplexer();

        /**
         * Destructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        ~Multiplexer();

    public:
        void setup(cref<sptr<Device>> device_);

        void destroy();

    public:
        bool allocate(const ptr<HORenderPass> renderPass_);

        void free(const ptr<HORenderPass> renderPass_);

    private:
        void dispatchNull(
            const non_owning_rptr<HORenderPass> renderPass_,
            const non_owning_rptr<RenderStagePass> stagePass_
        ) const;

        void dispatchAny(
            const non_owning_rptr<HORenderPass> renderPass_,
            const non_owning_rptr<RenderStagePass> stagePass_,
            const non_owning_rptr<SceneNodeModel> model_
        ) const;

    public:
        void attach(
            const non_owning_rptr<HORenderPass> renderPass_,
            const non_owning_rptr<RenderStagePass> stagePass_
        ) const;

        void dispatch(
            const non_owning_rptr<HORenderPass> renderPass_,
            const non_owning_rptr<RenderStagePass> stagePass_,
            const non_owning_rptr<SceneNodeModel> model_
        ) const;

        void detach(
            const non_owning_rptr<HORenderPass> renderPass_,
            const non_owning_rptr<RenderStagePass> stagePass_
        ) const;

    public:
        /**
         * Pushes a RenderStageNode to this Multiplexer
         *
         * @details The Multiplexer will take ownership of the RenderStageNode Instance.
         *  This will also effect the forwarding of invocations like `setup`, `destroy`...
         *
         * TODO: Check whether we want to support shared pointer for RenderStageNodes. This might reduce the resource cost, but will require some kind of setup/init/destroy flagging
         *
         * @param node_ The provided RenderStageNode
         */
        void push(mref<ptr<RenderStageNode>> node_);

        bool pop(cref<non_owning_rptr<RenderStageNode>> node_);

        [[deprecated]] void insert(const node_container_type::iterator where_, cref<sptr<RenderStageNode>> node_);

    private:
        constexpr static inline type_id null_type_id { 0ui64 };

        node_container_type _nodes;
        MultiplexerNodeMesh _mesh;
    };
}
