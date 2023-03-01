#include "SceneNodeHead.hpp"

#include "SceneNode.hpp"

using namespace hg::engine::scene;
using namespace hg;

SceneNodeHead::SceneNodeHead() noexcept :
    _nodeId(invalid_node_id {}),
    _cachedPtr(nullptr),
    _cacheVersion(invalid_node_version {}) {}

SceneNodeHead::SceneNodeHead(cref<SceneNodeId> nodeId_) noexcept :
    _nodeId(nodeId_),
    _cachedPtr(nullptr),
    _cacheVersion(invalid_node_version {}) {}

SceneNodeHead::SceneNodeHead(cref<this_type> other_) noexcept :
    _nodeId(other_._nodeId),
    _cachedPtr(nullptr),
    _cacheVersion(invalid_node_version {}) {}

SceneNodeHead::SceneNodeHead(mref<SceneNodeHead> other_) noexcept :
    _nodeId(_STD move(other_._nodeId)),
    _cachedPtr(_STD exchange(other_._cachedPtr, nullptr)),
    _cacheVersion(_STD exchange(other_._cacheVersion, 0)) {}

SceneNodeHead::~SceneNodeHead() noexcept = default;

ref<SceneNodeHead> SceneNodeHead::operator=(cref<this_type> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _nodeId = other_._nodeId;
        _cachedPtr = other_._cachedPtr;
        _cacheVersion = other_._cacheVersion;
    }

    return *this;
}

ref<SceneNodeHead> SceneNodeHead::operator=(mref<this_type> other_) noexcept {

    if (this != &other_) {
        _nodeId = _STD move(other_._nodeId);
        _cachedPtr = _STD exchange(other_._cachedPtr, nullptr);
        _cacheVersion = _STD exchange(other_._cacheVersion, invalid_node_version {});
    }

    return *this;
}

cref<SceneNodeId> SceneNodeHead::nodeId() const noexcept {
    return _nodeId;
}
