#include "SpartialSceneNode.hpp"

using namespace ember::engine::scene;
using namespace ember;

bool SpartialSceneNode::contains(SceneNode::const_reference_type other_) const noexcept {
    if (isLeaf()) {
        return false;
    }

    // TODO: Replace
    for (const auto& entry : _children) {
        auto resolved = entry.get();

        if (*resolved == other_) {
            return true;
        }

        if constexpr (type_trait::distinct_intersect) {
            if (resolved->intersects(other_))
                return resolved->contains(other_);
        } else {
            if (resolved->intersects(other_) &&
                resolved->contains(other_)) {
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

bool SpartialSceneNode::contains(cref<SceneNodeId> nodeId_) const noexcept {
    return contains_by_id<SceneNodeHeadContainer, SceneNodeId>(_children, nodeId_);
}

bool SpartialSceneNode::push(mref<SceneNodeHead> node_) {
    throw NotImplementedException();
}

pull_result_type SpartialSceneNode::pull(cref<SceneNodeId> nodeId_) noexcept {
    throw NotImplementedException();
}

bool SpartialSceneNode::pullChained(SceneNode::const_reference_type parent_, IN OUT ptr<SceneNodeHead> pos_) noexcept {
    throw NotImplementedException();
}

bool SpartialSceneNode::intersects(SceneNode::const_reference_type node_) const noexcept {
    return true;
}

bool SpartialSceneNode::intersectsFully(SceneNode::const_reference_type node_) const noexcept {
    return true;
}

bool SpartialSceneNode::intersectedFully(SceneNode::const_reference_type node_) const noexcept {
    return true;
}
