#include "NaturalSceneNode.hpp"

#include "SceneNodeFactory.hpp"

using namespace ember::engine::scene;
using namespace ember;

NaturalSceneNode::NaturalSceneNode() noexcept :
    SceneNodeSubBase<NaturalSceneNode>() {
    _state = SceneNodeState::eNatural;
}

bool NaturalSceneNode::contains(SceneNode::const_reference_type other_) const noexcept {

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

bool NaturalSceneNode::contains(cref<SceneNodeId> nodeId_) const noexcept {
    return contains_by_id<SceneNodeHeadContainer, SceneNodeId>(_children, nodeId_);
}

bool NaturalSceneNode::push(_Inout_ mref<SceneNodeCreateData> data_, _In_ const ptr<const SceneNodeFactory> factory_) {

    #ifdef _DEBUG
    if (_children.empty() && _children.unsafe_base() == nullptr) {
        throw _STD runtime_error("Node Container does not contain a valid or pre-allocated memory sequence.");
    }
    #endif

    if (isLeaf()) {

        // If this is empty, push payload to this node
        if (_payload.empty()) {
            _payload = _STD move(data_.payload);

            ++_size;
            return true;

        } else {

            const auto halfs = _bounding.extent() / 2.0F;
            const auto& center = _bounding.center();

            // If this is not empty, split this node into childs
            for (u8 i = 0; i < 8ui8; ++i) {

                const math::vec3 dir {
                    i & 0b0001 ? -1.F : 1.F,
                    i & 0b0010 ? -1.F : 1.F,
                    i & 0b0100 ? -1.F : 1.F
                };

                auto result = factory_->assembleNatural();

                // TODO: result.body->transform() = _transformation;
                result.body->bounding() = {
                    center + (halfs * dir),
                    halfs
                };

                _children.push(_STD move(result.head));
            }

            // Recursive push due to changed leaf state
            return push(_STD move(data_), factory_);
        }

    } else {

        for (auto& entry : _children) {
            auto resolved = entry.get();

            if (resolved->intersectsFully(data_)) {
                return resolved->push(_STD move(data_), factory_), ++_size;
            }
        }

        throw _STD runtime_error("Unable to sustain natural scene tree.");

    }
}

bool NaturalSceneNode::push(const ptr<SceneNodeCreateData> data_, const ptr<const SceneNodeFactory> factory_) {

    #ifdef _DEBUG
    if (_children.empty() && _children.unsafe_base() == nullptr) {
        throw _STD runtime_error("Node Container does not contain a valid or pre-allocated memory sequence.");
    }
    #endif

    if (isLeaf()) {

        // If this is empty, push payload to this node
        if (_payload.empty()) {
            _payload = _STD move(data_->payload);

            ++_size;
            return true;

        } else {

            const auto halfs = _bounding.extent() / 2.0F;
            const auto& center = _bounding.center();

            // If this is not empty, split this node into childs
            for (u8 i = 0; i < 8ui8; ++i) {

                const math::vec3 dir {
                    i & 0b0001 ? -1.F : 1.F,
                    i & 0b0010 ? -1.F : 1.F,
                    i & 0b0100 ? -1.F : 1.F
                };

                auto result = factory_->assembleNatural();

                // TODO: result.body->transform() = _transformation;
                result.body->bounding() = {
                    center + (halfs * dir),
                    halfs
                };

                _children.push(_STD move(result.head));
            }

            // Recursive push due to changed leaf state
            return push(data_, factory_);
        }

    } else {

        for (auto& entry : _children) {
            auto resolved = entry.get();

            if (resolved->intersectsFully(*data_)) {
                return resolved->push(data_, factory_), ++_size;
            }
        }

        throw _STD runtime_error("Unable to sustain natural scene tree.");

    }
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

bool NaturalSceneNode::intersects(cref<SceneNodeCreateData> data_) const noexcept {
    return _bounding.intersectsAaBb(data_.bounding);
}

bool NaturalSceneNode::intersectsFully(SceneNode::const_reference_type node_) const noexcept {
    return true;
}

bool NaturalSceneNode::intersectsFully(cref<SceneNodeCreateData> data_) const noexcept {
    return _bounding.containsAaBb(data_.bounding.extent(), data_.bounding.extent());
}

bool NaturalSceneNode::intersectedFully(SceneNode::const_reference_type node_) const noexcept {
    return true;
}
