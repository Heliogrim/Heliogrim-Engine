#include "SceneGraph.hpp"

#include <Engine.Common/Collection/Stack.hpp>

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

void SceneGraph::traversalBatchedSingle(u32 batchIdx_, cref<batched_consumer_fnc_type> consumer_,
    ptr<const SceneNodeHead> parent_) const {
    // Using stack instead of queue will cause graph to traverse primary vertical and secondary horizontal
    // We assume that memory coherency is stronger when traversing vertical
    stack<ptr<const SceneNodeHead>> backlog {};
    backlog.push(parent_);

    ref<EmberSceneNodeStorage> storage = *_storage;
    constexpr _STD nothrow_t nothrow {};

    while (!backlog.empty()) {
        DEBUG_ASSERT(backlog.top() != nullptr, "Cursor element should never be nullptr or undefined")

        // Important: pop() will invalidate top but, we dereference before we make reference to object, so indirection is not modified
        cref<SceneNodeHead> cursor = *backlog.top();
        backlog.pop();

        // Only move into sub-graph if consumer_ intersects and node isn't leaf
        auto payload = cursor.get(storage, nothrow);

        if (payload && consumer_(batchIdx_, *payload) && !payload->isLeaf()) {
            for (const auto& children = payload->children(); const auto& child : children) {
                backlog.push(&child);
            }
        }
    }
}

void SceneGraph::traversalBatchedPartition(u32 firstBatchIdx_, u32 lastBatchIdx_,
    cref<std::function<bool(u32, cref<SceneNode>)>> consumer_, ptr<const SceneNodeHead> parent_) const {

    if (firstBatchIdx_ == lastBatchIdx_) {
        traversalBatchedSingle(firstBatchIdx_, consumer_, parent_);
        return;
    }

    stack<ptr<const SceneNodeHead>> backlog {};
    backlog.push(parent_);

    ref<EmberSceneNodeStorage> storage = *_storage;
    constexpr _STD nothrow_t nothrow {};

    while (!backlog.empty() && firstBatchIdx_ != lastBatchIdx_) {
        DEBUG_ASSERT(backlog.top() != nullptr, "Cursor element should never be nullptr or undefined")

        // Important: pop() will invalidate top but, we dereference before we make reference to object, so indirection is not modified
        cref<SceneNodeHead> cursor = *backlog.top();
        backlog.pop();

        // Only move into sub-graph if consumer_ intersects and node isn't leaf
        auto payload = cursor.get(storage, nothrow);

        if (payload && consumer_(0, *payload) && !payload->isLeaf()) {
            for (const auto& children = payload->children(); const auto& child : children) {
                backlog.push(&child);
            }
        }

        /**
         *
         */
        if (backlog.size() > 1) {
            const u32 shareable = backlog.size() - 1;
            const u32 leftBatches = lastBatchIdx_ - firstBatchIdx_;

            const u32 partitions = MIN(shareable, leftBatches);
            const u32 perShare = leftBatches / partitions;

            for (u32 i = 0; i < partitions; ++i) {

                auto* cur = backlog.top();
                backlog.pop();

                if (perShare > 1) {

                    // scheduler::Scheduler::get().exec(scheduler::task::make_task(
                    //     [&, first = lastBatchIdx_ - perShare, last = lastBatchIdx_, consumer = consumer_, parent = cur
                    //     ]() {
                    //         traversalBatchedPartition(first, last, consumer, parent);
                    //     }, scheduler::task::TaskMask::eNormal,
                    //     engine::scheduler::ScheduleStageBarriers::eGraphicNodeCollectStrong,
                    //     engine::scheduler::ScheduleStageBarriers::eGraphicNodeCollectStrong));
                    // 
                    // lastBatchIdx_ -= perShare;

                } else {

                    // scheduler::Scheduler::get().exec(scheduler::task::make_task(
                    //     [&, idx = lastBatchIdx_--, consumer = consumer_, parent = cur]() {
                    //         traversalBatchedSingle(idx, consumer, parent);
                    //     }, scheduler::task::TaskMask::eNormal,
                    //     engine::scheduler::ScheduleStageBarriers::eGraphicNodeCollectStrong,
                    //     engine::scheduler::ScheduleStageBarriers::eGraphicNodeCollectStrong));
                }

            }
        }
    }

    while (!backlog.empty()) {

        auto* cursor = backlog.top();
        backlog.pop();

        traversalBatchedSingle(firstBatchIdx_, consumer_, cursor);
    }
}

void SceneGraph::traversalBatched(u32 maxBatches_, cref<std::function<bool(u32, cref<SceneNode>)>> consumer_) const {

    if (maxBatches_ <= 1) {
        traversalBatchedSingle(0, consumer_, &_root);
        return;
    }

    traversalBatchedPartition(0, maxBatches_ - 1, consumer_, &_root);
}
