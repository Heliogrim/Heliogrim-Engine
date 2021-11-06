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
         * Pushes an object onto this node or sub graph
         *
         * @author Julius
         * @date 04.11.2021
         *
         * @param data_ The data to construct from.
         * @param factory_ The factory to construct nodes.
         *
         * @returns True if it succeeds, false if it fails.
         */
        _Success_(return == true) bool push(_Inout_ mref<SceneNodeCreateData> data_,
            _In_ const ptr<const SceneNodeFactory> factory_);

        /**
         * Pushes an object onto this node or sub graph
         *
         * @author Julius
         * @date 04.11.2021
         *
         * @param data_ The data to construct from.
         * @param factory_ The factory to construct nodes.
         *
         * @returns True if it succeeds, false if it fails.
         */
        _Success_(return == true) bool push(_In_ const ptr<SceneNodeCreateData> data_,
            _In_ const ptr<const SceneNodeFactory> factory_);

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

        [[nodiscard]] bool intersects(cref<SceneNode> node_) const noexcept;

        [[nodiscard]] _Success_(return == true) bool intersects(_In_ cref<SceneNodeCreateData> data_) const noexcept;

        [[nodiscard]] bool intersectsFully(cref<SceneNode> node_) const noexcept;

        [[nodiscard]] _Success_(return == true) bool intersectsFully(
            _In_ cref<SceneNodeCreateData> data_) const noexcept;

        [[nodiscard]] bool intersectedFully(cref<SceneNode> node_) const noexcept;
    };
}
