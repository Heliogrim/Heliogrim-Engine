#include "ShadowSceneNode.hpp"

#include "SceneNodeHeadContainer.hpp"

using namespace ember::engine::scene;
using namespace ember;

bool ShadowSceneNode::contains(SceneNode::const_reference_type other_) const noexcept {
    if (isLeaf()) {
        return false;
    }

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

bool ShadowSceneNode::contains(cref<SceneNodeId> nodeId_) const noexcept {
    return contains_by_id<SceneNodeHeadContainer, SceneNodeId>(_children, nodeId_);
}

bool ShadowSceneNode::push(mref<SceneNodeHead> node_) {
    #ifdef _DEBUG
    if (_children.empty() && _children.unsafe_base() == nullptr) {
        throw _STD runtime_error("Node Container does not contain a valid or pre-allocated memory sequence.");
    }
    #endif

    // Container is full -> need to insert into descendant
    if (_children.size() >= type_trait::max_nodes_per_layer) {

        // Preference every other node than shadow node
        for (auto& entry : _children) {
            auto resolved = entry.get();

            if (resolved->isShadowNode())
                continue;

            if constexpr (type_trait::distinct_intersect) {
                if (resolved->intersectsFully(*node_.get())) {
                    return resolved->push(_STD move(node_));
                }
            } else {
                // TODO: consider memory pressure
                if (resolved->intersectsFully(*node_.get()) && resolved->push(_STD move(node_))) {
                    return true;
                }
            }
        }

        // Fallback to shadow node
        u64 lowestSize = ~0;
        container_type::iterator cur = _children.end();

        const container_type::iterator end = _children.end();
        container_type::iterator iter = _children.begin();

        for (; iter != end; ++iter) {
            if (!iter->get()->isShadowNode())
                continue;

            const auto testSize = iter->get()->size();
            if (testSize <= 1ui64) {
                cur = iter;
                break;
            }

            if (testSize < lowestSize) {
                cur = iter;
                lowestSize = testSize;
            }
        }

        // TODO: consider memory pressure
        return cur->get()->push(_STD move(node_));
    }

    // Container has some space left -> insert directly
    return _children.push(_STD move(node_));
}

pull_result_type ShadowSceneNode::pull(cref<SceneNodeId> nodeId_) noexcept {

    if (isLeaf()) {
        return {};
    }

    auto iter = _children.begin();
    const auto last = _children.end();

    // Check for direct child node
    for (; iter != last; ++iter) {
        if (iter->nodeId() == nodeId_)
            break;
    }

    // Check for indirect childs in sub-graph
    if (iter == last) {
        pull_result_type result {};
        for (iter = _children.begin(); iter != last; ++iter) {
            if (result = iter->get()->pull(nodeId_)) {
                return result;
            }
        }
    }

    // Guard child removal, if reference is not contained in this or sub-graph
    if (iter == last) {
        return {};
    }

    // TODO:
    ASSERT(iter->get()->isShadowNode())

    //
    auto& head { *iter };
    pull_result_type result = pull_result_type::make<SceneNodeHead>(_STD move(head));

    if (!result.value->get()->pullChained(*this, iter.pointer())) {
        #ifdef _DEBUG
        // Attention: This will invalidate the target iterator cause we modify the underlying container memory sequence
        ASSERT(_children.remove(iter))
        #else
        _children.remove(iter)
        #endif
    }

    return result;
}

bool ShadowSceneNode::pullChained(SceneNode::const_reference_type parent_, IN OUT ptr<SceneNodeHead> pos_) noexcept {
    if (isLeaf()) {
        return false;
    }

    // In-Place replace with [biggest or leaf] shadow node might be best
    container_type::iterator target = _children.end();
    u64 maxSize = 0;

    auto iter = _children.begin();
    const auto last = _children.end();

    // Check for shadow nodes => Replacing Shadow Node with Shadow Node
    for (; iter != last; ++iter) {
        if (!iter->get()->isShadowNode())
            continue;

        if (iter->get()->isLeaf()) {
            target = iter;
            break;
        }

        const auto testSize = iter->get()->size();
        if (testSize >= maxSize) {
            target = iter;
            maxSize = testSize;
        }
    }

    // When no other shadow node is available, check for other node
    if (target == last) {
        throw NotImplementedException();
    }

    // TODO:
    ASSERT(target->get()->isShadowNode())

    // Replace in-place at parent iterator position
    /**
     * A
     * | \
     * |  \
     * B   C
     *    / \       /\ {Pushing Reference Up -> Forward}
     *   D   E
     */
    using value_type = container_type::value_type;

    pos_->value_type::~value_type();
    new(pos_) value_type { _STD move(*target) };

    // Check whether container element was replace or needs to be removed
    SceneNode::reference_type targetNode { *pos_->get() };

    if (!targetNode.pullChained(*this, target.pointer())) {
        #ifdef _DEBUG
        // Attention: This will invalidate the target iterator cause we modify the underlying container memory sequence
        ASSERT(_children.remove(target))
        #else
        _children.remove(target);
        #endif
    }

    // Push container to in-place target
    /**
     * A
     * | \
     * |  \
     * B   E[C]
     *    / \       \/ {Pushing Container Down -> Replace}
     *   D   -[E]
     */
    targetNode.children() = _STD move(_children);
    return true;
}

bool ShadowSceneNode::intersects(SceneNode::const_reference_type node_) const noexcept {
    // Any node_ will intersect with shadow node
    return true;
}

bool ShadowSceneNode::intersectsFully(SceneNode::const_reference_type node_) const noexcept {
    // Any node_ will intersect with shadow node
    return true;
}

bool ShadowSceneNode::intersectedFully(SceneNode::const_reference_type node_) const noexcept {
    // Any node_ will intersect with shadow node
    return true;
}
