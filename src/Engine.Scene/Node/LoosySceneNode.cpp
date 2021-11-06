#include "LoosySceneNode.hpp"

using namespace ember::engine::scene;
using namespace ember;

bool LoosySceneNode::contains(SceneNode::const_reference_type other_) const noexcept {
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

bool LoosySceneNode::contains(cref<SceneNodeId> nodeId_) const noexcept {
    return contains_by_id<SceneNodeHeadContainer, SceneNodeId>(_children, nodeId_);
}

bool LoosySceneNode::push(mref<SceneNodeCreateData> data_, const ptr<const SceneNodeFactory> factory_) {
    throw NotImplementedException();
}

bool LoosySceneNode::push(const ptr<SceneNodeCreateData> data_, const ptr<const SceneNodeFactory> factory_) {
    throw NotImplementedException();
}

pull_result_type LoosySceneNode::pull(cref<SceneNodeId> nodeId_) noexcept {
    throw NotImplementedException();
}

bool LoosySceneNode::pullChained(SceneNode::const_reference_type parent_, IN OUT ptr<SceneNodeHead> pos_) noexcept {
    throw NotImplementedException();
}

bool LoosySceneNode::intersects(SceneNode::const_reference_type node_) const noexcept {
    return true;
}

bool LoosySceneNode::intersects(cref<SceneNodeCreateData> data_) const noexcept {
    return false;
}

bool LoosySceneNode::intersectsFully(SceneNode::const_reference_type node_) const noexcept {
    return true;
}

bool LoosySceneNode::intersectsFully(cref<SceneNodeCreateData> data_) const noexcept {
    return false;
}

bool LoosySceneNode::intersectedFully(SceneNode::const_reference_type node_) const noexcept {
    return true;
}
