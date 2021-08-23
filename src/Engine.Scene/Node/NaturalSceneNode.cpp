#include "NaturalSceneNode.hpp"

using namespace ember::engine::scene;
using namespace ember;

bool NaturalSceneNode::contains(SceneNode::const_reference_type node_) const noexcept {
    if (isLeaf()) {
        return false;
    }

    // TODO: Replace
    for (const auto& entry : _children) {
        auto resolved = entry.get();

        if (*resolved == node_) {
            return true;
        }

        if constexpr (type_trait::distinct_intersect) {
            if (resolved->intersects(node_))
                return resolved->contains(node_);
        } else {
            if (resolved->intersects(node_) &&
                resolved->contains(node_)) {
                return true;
            }
        }
    }

    return false;
}

template <typename ContainerType_, typename NodeIdType_>
FORCE_INLINE constexpr bool contains_by_id(
    cref<_STD remove_reference_t<ContainerType_>> container_,
    NodeIdType_ id_
) noexcept {
    if (container_.empty()) {
        return false;
    }

    // First check for direct member
    for (const auto& entry : container_) {
        if (entry.nodeId() == id_) {
            return true;
        }
    }

    // Check for indirect member
    for (const auto& entry : container_) {
        auto resolved = entry.get();
        if (resolved->contains(id_)) {
            return true;
        }
    }

    return false;
}

bool NaturalSceneNode::contains(cref<SceneNodeId> nodeId_) const noexcept {
    return contains_by_id<SceneNodeHeadContainer, SceneNodeId>(_children, nodeId_);
}

bool NaturalSceneNode::push(mref<SceneNodeHead> node_) {
    throw NotImplementedException();
}

pull_result_type NaturalSceneNode::pull(cref<SceneNodeId> nodeId_) noexcept {
    throw NotImplementedException();
}

bool NaturalSceneNode::pullChained(SceneNode::const_reference_type parent_, IN OUT ptr<SceneNodeHead> pos_) noexcept {
    throw NotImplementedException();
}

bool NaturalSceneNode::intersects(SceneNode::const_reference_type node_) const noexcept {
    return true;
}

bool NaturalSceneNode::intersectsFully(SceneNode::const_reference_type node_) const noexcept {
    return true;
}

bool NaturalSceneNode::intersectedFully(SceneNode::const_reference_type node_) const noexcept {
    return true;
}
