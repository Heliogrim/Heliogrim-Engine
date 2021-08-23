#include "SceneNodeHead.hpp"

#include "SceneNode.hpp"
#include "../Storage/EmberSceneNodeStorage.hpp"

using namespace ember::engine::scene;
using namespace ember;

SceneNodeHead::SceneNodeHead() noexcept :
    _nodeId(invalid_node_id {}),
    _cachedPtr(nullptr),
    _cacheVersion(invalid_node_version {}) {}

SceneNodeHead::SceneNodeHead(cref<SceneNodeId> nodeId_) noexcept :
    _nodeId(nodeId_),
    _cachedPtr(nullptr),
    _cacheVersion(invalid_node_version {}) {}

SceneNodeHead::SceneNodeHead(const_reference_type other_) noexcept :
    _nodeId(other_._nodeId),
    _cachedPtr(nullptr),
    _cacheVersion(invalid_node_version {}) {}

SceneNodeHead::SceneNodeHead(mref<SceneNodeHead> other_) noexcept :
    _nodeId(_STD move(other_._nodeId)),
    _cachedPtr(_STD exchange(other_._cachedPtr, nullptr)),
    _cacheVersion(_STD exchange(other_._cacheVersion, 0)) {}

SceneNodeHead::~SceneNodeHead() noexcept = default;

SceneNodeHead::reference_type SceneNodeHead::operator=(const_reference_type other_) noexcept {
    _nodeId = other_._nodeId;
    _cachedPtr = other_._cachedPtr;
    _cacheVersion = other_._cacheVersion;

    return *this;
}

SceneNodeHead::reference_type SceneNodeHead::operator=(mref<value_type> other_) noexcept {
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

ptr<SceneNode> SceneNodeHead::get(cref<EmberSceneNodeStorage> storage_) const {
    if (_cachedPtr == nullptr || _cachedPtr->_version != _cacheVersion) {
        _cachedPtr = storage_.get(_nodeId);

        if (_cachedPtr == nullptr) {
            throw _STD bad_exception {};
        }

        _cacheVersion = _cachedPtr->_version;
    }

    return _cachedPtr;
}

ptr<SceneNode> SceneNodeHead::get(cref<EmberSceneNodeStorage> storage_, _STD nothrow_t) const noexcept {
    if (_cachedPtr == nullptr || _cachedPtr->_version != _cacheVersion) {
        _cachedPtr = storage_.get(_nodeId);

        if (_cachedPtr == nullptr) {
            return nullptr;
        }

        _cacheVersion = _cachedPtr->_version;
    }

    return _cachedPtr;
}

ptr<SceneNode> SceneNodeHead::get() const noexcept {
    return get(*storage, _STD nothrow_t {});
}
