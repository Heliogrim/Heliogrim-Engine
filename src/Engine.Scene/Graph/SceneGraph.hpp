#pragma once

#ifdef _DEBUG
#include <cassert>
#endif

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Collection/Stack.hpp>

#include "../Node/SceneNodeConsumer.hpp"
#include "../Node/SceneNodeHead.hpp"
#include "../Node/Traits.hpp"
#include "../Storage/SceneElementStorage.hpp"
#include "../Storage/SceneNodeStorage.hpp"

#include "../Node/LoosySceneNode.hpp"
#include "../Node/NaturalSceneNode.hpp"
#include "../Node/ShadowSceneNode.hpp"
#include "../Node/SpartialSceneNode.hpp"

namespace ember::engine::scene {

    /**
     * Forward Declaration
     */
    template <class PayloadType_>
    class MutableSceneGraph;

    template <class PayloadType_>
    class SceneGraph {
    public:
        using this_type = SceneGraph<PayloadType_>;

        using head_type = SceneNodeHead;
        using node_type = SceneNode<PayloadType_>;

        using traits = scene_traits<node_type, PayloadType_>;

        using node_storage_type = scene_node_storage<node_type, SceneNodeId>;
        using element_storage_type = SceneElementStorage<PayloadType_, traits>;

        using root_type = head_type;
        using batched_consumer_fnc_type = _STD function<bool(u32, const ptr<node_type>)>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 15.08.2021
         */
        SceneGraph() = delete;

        /**
         * Value Move Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param root_ The root.
         * @param nodeStorage_ The storage instance where to store graph nodes
         * @param elementStorage_ The storage instance where to store node elements
         */
        SceneGraph(
            mref<_STD remove_reference_t<root_type>> root_,
            mref<sptr<node_storage_type>> nodeStorage_,
            mref<sptr<element_storage_type>> elementStorage_) noexcept :
            _root(_STD move(root_)),
            _nodeStorage(_STD move(nodeStorage_)),
            _elementStorage(_STD move(elementStorage_)) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 15.08.2021
         */
        SceneGraph(cref<this_type>) noexcept = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         */
        SceneGraph(mref<this_type> other_) noexcept :
            _root(_STD move(other_._root)),
            _nodeStorage(_STD move(other_._nodeStorage)),
            _elementStorage(_STD move(other_._elementStorage)) {}

        /**
         * Destructor
         *
         * @author Julius
         * @date 15.08.2021
         */
        ~SceneGraph() noexcept = default;

    public:
        /**
         * Copy Assignment
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @returns A shallow copy of this.
         */
        ref<this_type> operator=(cref<this_type>) = delete;

        /**
         * Move Assignment
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        ref<this_type> operator=(mref<this_type> other_) noexcept {

            if (_STD addressof(other_) != this) {
                _root = _STD move(other_._root);
                _nodeStorage = _STD move(other_._nodeStorage);
                _elementStorage = _STD move(other_._elementStorage);
            }

            return *this;
        }

    protected:
        /**
         * Scene Graph Root
         */
        root_type _root;

    public:
        [[nodiscard]] cref<decltype(_root)> root() const noexcept {
            return _root;
        }

        [[nodiscard]] ref<decltype(_root)> root() noexcept {
            return _root;
        }

    protected:
        /**
         * Scene Graph Storage
         */
        sptr<node_storage_type> _nodeStorage;

    public:
        [[nodiscard]] cref<sptr<node_storage_type>> getNodeStorage() const noexcept {
            return _nodeStorage;
        }

    protected:
        /**
         * Scene Element Storage
         */
        sptr<element_storage_type> _elementStorage;

    public:
        [[nodiscard]] cref<sptr<element_storage_type>> getElementStorage() const noexcept {
            return _elementStorage;
        }

    public:
        /**
         * Converts this to a mutable scene graph
         *
         * @author Julius
         * @date 01.11.2021
         *
         * @returns A const reference to this as mutable scene graph
         */
        [[nodiscard]] cref<MutableSceneGraph<PayloadType_>> asMutable() const noexcept {
            return *static_cast<ptr<const MutableSceneGraph<PayloadType_>>>(this);
        }

        /**
         * Converts this to a mutable scene graph
         *
         * @author Julius
         * @date 01.11.2021
         *
         * @returns A reference to this as mutable scene graph
         */
        [[nodiscard]] ref<MutableSceneGraph<PayloadType_>> asMutable() noexcept {
            return *static_cast<ptr<MutableSceneGraph<PayloadType_>>>(this);
        }

    public:
        /**
         * Traversals the graph with the given consumer
         *
         * @tparam Consumer Type of the consumer.
         * @param  consumer_ The consumer.
         */
        template <typename Consumer> requires IsSceneNodeConsumer<Consumer, node_type>
        void traversal(cref<Consumer> consumer_) const {
            return traversal(static_cast<_STD function<bool(const ptr<node_type>)>>(consumer_));
        }

        /**
         * Traversals the graph with the given consumer
         *
         * @author Julius
         * @date 09.01.2021
         *
         * @param  consumer_ The consumer.
         */
        void traversal(cref<_STD function<bool(const ptr<node_type>)>> consumer_) const {
            // Using stack instead of queue will cause graph to traverse primary vertical and secondary horizontal
            // We assume that memory coherency is stronger when traversing vertical
            stack<ptr<const SceneNodeHead>, Vector<ptr<const SceneNodeHead>>> backlog {};
            backlog.push(&_root);

            /*
             *     0
             *    / \
             *   1   2
             *  / \ / \
             * 3  4 5  6
             */

            // Using Forward Iterator
            // ----------------------
            // null -> [0] {} :: {}
            // [0]  -> [] {0} :: {1, 2}
            // [0]  -> [1, 2] {0} :: {}
            // [2]  -> [1] {2} :: {5, 6}
            // [2]  -> [1, 5, 6] {2} :: {}
            // [6]  -> [1, 5] {6} :: {~}
            // [6]  -> [1, 5] {6} :: {}
            // [5]  -> [1] {5} :: {~}
            // [5]  -> [1] {5} :: {}
            // [1]  -> [] {1} :: {3, 4}
            // [1]  -> [3, 4] {1} :: {}
            // [4]  -> [3] {4} :: {~}
            // [4]  -> [3] {4} :: {}
            // [3]  -> [] {3} :: {~}
            // [3]  -> [] {3} :: {}
            // null -> [] {} :: {}

            auto storage { _nodeStorage.get() };
            while (!backlog.empty()) {
                DEBUG_ASSERT(backlog.top() != nullptr, "Cursor element should never be nullptr or undefined")

                // Important: pop() will invalidate top but, we dereference before we make reference to object, so indirection is not modified
                cref<SceneNodeHead> cursor = *backlog.top();
                backlog.pop();

                // Only move into sub-graph if consumer_ intersects and node isn't leaf
                auto* payload = cursor.get(storage);

                if (payload && consumer_(payload) && !payload->isLeaf()) {

                    const auto& children = payload->children();
                    const auto rend { children.rend() };
                    for (auto it { children.rbegin() }; it != rend; ++it) {
                        backlog.push(it.pointer());
                    }
                }
            }
        }

    private:
        void traversalBatchedSingle(u32 batchIdx_, cref<batched_consumer_fnc_type> consumer_,
            ptr<const SceneNodeHead> parent_) const {

            /**
             *
             */
            const auto storage { _nodeStorage.get() };

            // Using stack instead of queue will cause graph to traverse primary vertical and secondary horizontal
            // We assume that memory coherency is stronger when traversing vertical
            Vector<ptr<const SceneNodeHead>> backlogContainer {};

            /*
            // prepare stack container with `min(value, 64)`
            const auto depth { parent_->get(storage)->depth(storage) };
            backlogContainer.reserve(_STD min(traits::max_childs_per_node * depth, 64ui64));
             */

            stack<ptr<const SceneNodeHead>, Vector<ptr<const SceneNodeHead>>> backlog {};
            backlog.push(parent_);

            while (!backlog.empty()) {
                //DEBUG_ASSERT(backlog.top() != nullptr, "Cursor element should never be nullptr or undefined")

                // Important: pop() will invalidate top but, we dereference before we make reference to object, so indirection is not modified
                cref<SceneNodeHead> cursor = *backlog.top();
                backlog.pop();

                // Only move into sub-graph if consumer_ intersects and node isn't leaf
                auto* payload = cursor.get(storage);

                if (
                    payload &&
                    consumer_(batchIdx_, payload) &&
                    !payload->isLeaf() &&
                    payload->inclusiveSize() != payload->exclusiveSize()
                ) {

                    const auto& children = payload->children();
                    const auto rend { children.rend() };
                    for (auto it { children.rbegin() }; it != rend; ++it) {
                        backlog.push(it.pointer());
                    }
                }
            }
        }

        void traversalBatchedPartition(u32 firstBatchIdx_, u32 lastBatchIdx_,
            cref<_STD function<bool(u32, const ptr<node_type>)>> consumer_, ptr<const SceneNodeHead> parent_) const {

            if (firstBatchIdx_ == lastBatchIdx_) {
                traversalBatchedSingle(firstBatchIdx_, consumer_, parent_);
                return;
            }

            stack<ptr<const SceneNodeHead>, Vector<ptr<const SceneNodeHead>>> backlog {};
            backlog.push(parent_);

            auto storage { _nodeStorage.get() };
            while (!backlog.empty() && firstBatchIdx_ != lastBatchIdx_) {
                DEBUG_ASSERT(backlog.top() != nullptr, "Cursor element should never be nullptr or undefined")

                // Important: pop() will invalidate top but, we dereference before we make reference to object, so indirection is not modified
                cref<SceneNodeHead> cursor = *backlog.top();
                backlog.pop();

                // Only move into sub-graph if consumer_ intersects and node isn't leaf
                auto* payload = cursor.get(storage);

                if (
                    payload &&
                    consumer_(0, payload) &&
                    !payload->isLeaf()
                ) {
                    const auto& children = payload->children();
                    const auto rend { children.rend() };
                    for (auto it { children.rbegin() }; it != rend; ++it) {
                        backlog.push(it.pointer());
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

    public:
        template <typename BatchConsumer> requires IsSceneNodeBatchConsumer<BatchConsumer, node_type>
        void traversalBatched(u32 maxBatches_, cref<BatchConsumer> consumer_) const {
            return traversalBatched(maxBatches_,
                static_cast<_STD function<bool(u32, const ptr<node_type>)>>(consumer_));
        }

        void traversalBatched(u32 maxBatches_, cref<_STD function<bool(u32, const ptr<node_type>)>> consumer_) const {

            if (maxBatches_ <= 1) {
                traversalBatchedSingle(0, consumer_, &_root);
                return;
            }

            traversalBatchedPartition(0, maxBatches_ - 1, consumer_, &_root);
        }

    };

}
