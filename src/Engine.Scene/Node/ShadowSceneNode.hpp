#pragma once

#include "SceneNodeSubBase.hpp"

namespace ember::engine::scene {
    class ShadowSceneNode final :
        public SceneNodeSubBase<ShadowSceneNode> {
    public:
        using type_trait = scene_node_traits<SceneNodeState::eShadow>;

        using base_type = SceneNode;

        using value_type = ShadowSceneNode;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Query if this is shadow node
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns True if shadow node, false if not.
         */
        [[nodiscard]] constexpr bool isShadowNode() const noexcept {
            return true;
        }

    public:
        /**
         * Query if this contains the given id_ as descendant
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  nodeId_ The cref&lt;SceneNodeId&gt; to test for containment.
         *
         * @returns True if the object is in this collection, false if not.
         */
        [[nodiscard]] bool contains(cref<SceneNodeId> nodeId_) const noexcept;

        /**
         * Query if this contains the given other_ as descendant
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  other_ The SceneNode::const_reference_type to test for containment.
         *
         * @returns True if the object is in this collection, false if not.
         */
        [[nodiscard]] bool contains(SceneNode::const_reference_type other_) const noexcept;

        /**
         * Pushes an object onto this node container
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  node_ The node to push.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool push(mref<SceneNodeHead> node_);

        /**
         * Pulls the given nodeId_
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  nodeId_ Identifier for the node.
         *
         * @returns A pull_result_type.
         */
        [[nodiscard]] pull_result_type pull(cref<SceneNodeId> nodeId_) noexcept;

        /**
         * Pulls this chained
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  parent_ The parent.
         * @param  pos_ The position.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool pullChained(
            SceneNode::const_reference_type parent_,
            IN OUT ptr<SceneNodeHead> pos_
        ) noexcept;

        [[nodiscard]] bool intersects(SceneNode::const_reference_type node_) const noexcept;

        [[nodiscard]] bool intersectsFully(SceneNode::const_reference_type node_) const noexcept;

        [[nodiscard]] bool intersectedFully(SceneNode::const_reference_type node_) const noexcept;
    };
}
