#include "SceneGraph.hpp"

#include <Engine.Common/Collection/List.hpp>

#ifdef _DEBUG
#include <cassert>
#endif

#include "MutableSceneGraph.hpp"

using namespace ember::engine::scene;
using namespace ember;

RefSceneGraph::RefSceneGraph(
    ref<_STD remove_reference_t<root_type>> root_,
    ref<_STD remove_reference_t<storage_type>> storage_) noexcept (
    _STD is_nothrow_copy_constructible_v<root_type> &&
    _STD is_nothrow_copy_constructible_v<storage_type>
) :
    _root(root_),
    _storage(storage_) {}

RefSceneGraph::RefSceneGraph(
    mref<_STD remove_reference_t<root_type>> root_,
    mref<_STD remove_reference_t<storage_type>> storage_) noexcept (
    _STD is_nothrow_move_constructible_v<root_type> &&
    _STD is_nothrow_move_constructible_v<storage_type>
) :
    _root(root_),
    _storage(storage_) {}

RefSceneGraph::~RefSceneGraph() noexcept = default;

cref<_STD remove_reference_t<RefSceneGraph::root_type>> RefSceneGraph::root() const noexcept {
    return _root;
}

cref<_STD remove_reference_t<RefSceneGraph::root_type>> RefSceneGraph::root() noexcept {
    return _root;
}

void RefSceneGraph::traversal(cref<std::function<bool(cref<SceneNode>)>> consumer_) const {
    // Using stack instead of queue will cause graph to traverse primary vertical and secondary horizontal
    // We assume that memory coherency is stronger when traversing vertical
    stack<ptr<const SceneNodeHead>> backlog {};
    backlog.push(&_root);

    constexpr _STD nothrow_t nothrow {};

    while (!backlog.empty()) {
        DEBUG_ASSERT(backlog.top() != nullptr, "Cursor element should never be nullptr or undefined")

        // Important: pop() will invalidate top but, we dereference before we make reference to object, so indirection is not modified
        cref<SceneNodeHead> cursor = *backlog.top();
        backlog.pop();

        // Only move into sub-graph if consumer_ intersects and node isn't leaf
        auto payload = cursor.get(_storage, nothrow);

        if (payload && consumer_(*payload) && !payload->isLeaf()) {
            for (const auto& children = payload->children(); const auto& child : children) {
                backlog.push(&child);
            }
        }
    }
}

SceneGraph::SceneGraph(
    cref<_STD remove_reference_t<root_type>> root_,
    cref<_STD remove_reference_t<storage_type>> storage_) noexcept(
    _STD is_nothrow_copy_constructible_v<root_type> &&
    _STD is_nothrow_copy_constructible_v<storage_type>
) :
    _root(root_),
    _storage(storage_) {}

SceneGraph::SceneGraph(
    mref<_STD remove_reference_t<root_type>> root_,
    mref<_STD remove_reference_t<storage_type>> storage_) noexcept(
    _STD is_nothrow_move_constructible_v<root_type> &&
    _STD is_nothrow_move_constructible_v<storage_type>
) :
    _root(_STD move(root_)),
    _storage(_STD move(storage_)) {}

SceneGraph::~SceneGraph() noexcept = default;

SceneGraph::reference_type SceneGraph::operator=(const_reference_type other_) noexcept {
    if (this != _STD addressof(other_)) {
        _root = other_._root;
        _storage = other_._storage;
    }

    return *this;
}

SceneGraph::reference_type SceneGraph::operator=(mref<value_type> other_) noexcept {
    if (this != _STD addressof(other_)) {
        _root = _STD move(other_._root);
        _storage = _STD move(other_._storage);
    }

    return *this;
}

cref<_STD remove_reference_t<SceneGraph::root_type>> SceneGraph::root() const noexcept {
    return _root;
}

ref<_STD remove_reference_t<SceneGraph::root_type>> SceneGraph::root() noexcept {
    return _root;
}

cref<SceneGraph::storage_type> SceneGraph::storage() const noexcept {
    return _storage;
}

RefSceneGraph SceneGraph::asRefSceneGraph() const {
    return RefSceneGraph {
        *const_cast<ptr<SceneNodeHead>>(&_root),
        *_storage.get()
    };
}

RefSceneGraph SceneGraph::asRefSceneGraph() {
    return RefSceneGraph {
        _root,
        *_storage.get()
    };
}

cref<MutableSceneGraph> SceneGraph::asMutable() const noexcept {
    return *static_cast<ptr<const MutableSceneGraph>>(this);
}

ref<MutableSceneGraph> SceneGraph::asMutable() noexcept {
    return *static_cast<ptr<MutableSceneGraph>>(this);
}

void SceneGraph::traversal(cref<_STD function<bool(cref<SceneNode>)>> consumer_) const {
    // Using stack instead of queue will cause graph to traverse primary vertical and secondary horizontal
    // We assume that memory coherency is stronger when traversing vertical
    stack<ptr<const SceneNodeHead>> backlog {};
    backlog.push(&_root);

    ref<EmberSceneNodeStorage> storage = *_storage;
    constexpr _STD nothrow_t nothrow {};

    while (!backlog.empty()) {
        DEBUG_ASSERT(backlog.top() != nullptr, "Cursor element should never be nullptr or undefined")

        // Important: pop() will invalidate top but, we dereference before we make reference to object, so indirection is not modified
        cref<SceneNodeHead> cursor = *backlog.top();
        backlog.pop();

        // Only move into sub-graph if consumer_ intersects and node isn't leaf
        auto payload = cursor.get(storage, nothrow);

        if (payload && consumer_(*payload) && !payload->isLeaf()) {
            for (const auto& children = payload->children(); const auto& child : children) {
                backlog.push(&child);
            }
        }
    }
}
