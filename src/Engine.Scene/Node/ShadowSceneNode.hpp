#pragma once

#include "SceneNodeSubBase.hpp"

namespace ember::engine::scene {

    template <class PayloadType_>
    class ShadowSceneNode final :
        public SceneNodeSubBase<PayloadType_, ShadowSceneNode<PayloadType_>> {
    public:
        using type_trait = scene_node_traits<SceneNodeState::eShadow>;

        using this_type = ShadowSceneNode<PayloadType_>;
        using underlying_type = SceneNodeSubBase<PayloadType_, this_type>;
        using base_type = SceneNode<PayloadType_>;

        using traits = typename base_type::traits;

    public:
        /**
         * Query if this is shadow node
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns True if shadow node, false if not.
         */
        [[nodiscard]] constexpr bool isShadowNode() const noexcept {
            return true;
        }

    public:
        /**
         * Query if this contains the given id_ as descendant
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  nodeId_ The cref&lt;SceneNodeId&gt; to test for containment.
         *
         * @returns True if the object is in this collection, false if not.
         */
        [[nodiscard]] bool contains(cref<SceneNodeId> nodeId_) const noexcept {

            #if FALSE
            if (base_type::_children.empty()) {
                return false;
            }

            // First check for direct member
            for (const auto& entry : base_type::_children) {
                if (entry.nodeId() == nodeId_) {
                    return true;
                }
            }

            // Check for indirect member
            for (const auto& entry : base_type::_children) {
                auto resolved = entry.get();
                if (resolved->contains(nodeId_)) {
                    return true;
                }
            }

            return false;
            #endif

            throw NotImplementedException {};
        }

        /**
         * Query if this contains the given other_ as descendant
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  other_ The node to test for containment.
         *
         * @returns True if the object is in this collection, false if not.
         */
        [[nodiscard]] bool contains(cref<base_type> other_) const noexcept {

            #if FALSE

            if (base_type::isLeaf()) {
                return false;
            }

            for (const auto& entry : base_type::_children) {
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

            #endif

            throw NotImplementedException {};
        }

        template <class FactoryType_>
        bool pushLeaf(const ptr<PayloadType_> element_, const ptr<const FactoryType_> factory_,
            const SceneNodePath path_) {
            /**
             * Ensure current leaf has element storage
             */
            if (!base_type::_elements) {
                base_type::_elements = factory_->getElementStorage()->acquire();
            }

            /**
             * Store element to current leaf
             */
            const auto index { base_type::_elementCount++ };
            if (index > traits::max_elements_per_leaf) {
                return --base_type::_elementCount, false;
            }

            ++base_type::_inclusiveElementCount;
            base_type::_elements[index] = element_;

            // Store path
            base_type::_elements[index]->setSceneNodePath(path_);

            return true;
        }

        bool popLeaf(const ptr<PayloadType_> element_) {
            /**
             * Ensure current leaf has elements
             */
            if (!base_type::_elements || !base_type::_elementCount) {
                return false;
            }

            /**
             * Store element to current leaf
             */
            auto idx { base_type::_elementCount - 1u };
            for (;
                idx < base_type::_elementCount &&
                base_type::_elements[idx] != element_;
                --idx
            ) { }

            /**
             * Ensure element was found
             */
            if (base_type::_elements[idx] != element_) {
                return false;
            }

            /**
             * Override target element with current last to release
             */
            base_type::_elements[idx] = base_type::_elements[--base_type::_elementCount];
            return --base_type::_inclusiveElementCount, true;
        }

        template <class FactoryType_>
        void leafToNode(cref<math::Bounding> boundary_, const ptr<const FactoryType_> factory_) {
            /**
             * Acquire new scene nodes as childs
             */
            // auto assembled { factory_->assembleNaturals(boundary_) };
            auto assembled { factory_->assembleShadows() };

            /**
             * Range base push to scene node head container
             */
            base_type::_children.push(assembled.heads, assembled.heads + traits::max_childs_per_node);
        }

        template <class FactoryType_>
        bool push(const ptr<PayloadType_> element_, cref<math::Bounding> boundary_,
            const ptr<const FactoryType_> factory_, SceneNodePath forwardPath_) {

            if (base_type::isLeaf()) {
                /**
                 * Check whether this is suitable leaf or we need to transform the leaf to node
                 */
                if (base_type::_elementCount <= traits::max_elements_per_leaf - 1u) {
                    return pushLeaf(element_, factory_, forwardPath_.inversed());
                }

                decltype(this->_elements) elements { _STD exchange(base_type::_elements, nullptr) };
                decltype(this->_elementCount) elementCount { _STD exchange(base_type::_elementCount, 0) };
                this->_inclusiveElementCount -= elementCount;

                /**
                 *
                 */
                leafToNode<FactoryType_>(boundary_, factory_);

                /**
                 *
                 */
                for (auto i = 0; i < elementCount; ++i) {
                    auto boundary { traits::getBounding(elements[i]) };
                    push(elements[i], boundary, factory_, forwardPath_);
                }

                /**
                 *
                 */
                return push(element_, boundary_, factory_, forwardPath_);
            }

            /**
             * Check non-shadow children
             */
            using cache_type = _STD pair<SceneNodePath::decision_type, ptr<base_type>>;
            cache_type cache[traits::max_childs_per_node];
            u64 cached { 0 };

            for (SceneNodePath::decision_type dt { 0u }; dt < base_type::_children.size(); ++dt) {

                auto& child { base_type::_children[dt] };
                auto* node { child.get(factory_->getNodeStorage()) };

                if (node->isShadowNode()) {
                    cache[cached++] = _STD make_pair(dt, node);
                    continue;
                }

                if (node->intersectsFully(boundary_)) {
                    return node->push(element_, boundary_, factory_, forwardPath_.push(dt)), ++
                        base_type::_inclusiveElementCount;
                }

            }

            /**
             * Check shadow children | Find lowest inclusive element count
             */
            decltype(this->_inclusiveElementCount) lowest { ~0ui64 };
            cache_type inclusiveNode {};
            cache_type exclusiveNode {};

            while (cached) {
                auto cur { cache[--cached] };

                if (cur.second->inclusiveSize() < lowest) {
                    inclusiveNode = cur;
                    lowest = cur.second->inclusiveSize();
                }

                if (cur.second->isLeaf() && cur.second->exclusiveSize() > 0 && cur.second->exclusiveSize() <
                    traits::max_elements_per_leaf) {
                    exclusiveNode = cur;
                }
            }

            if (exclusiveNode.second) {
                return exclusiveNode.second->push(element_, boundary_, factory_,
                        forwardPath_.push(exclusiveNode.first)),
                    ++base_type::_inclusiveElementCount;
            }

            return inclusiveNode.second->push(element_, boundary_, factory_,
                    forwardPath_.push(inclusiveNode.first)),
                ++base_type::_inclusiveElementCount;

            #if FALSE

            #ifdef _DEBUG
            if (base_type::_children.empty() && base_type::_children.unsafe_base() == nullptr) {
                throw _STD runtime_error("Node Container does not contain a valid or pre-allocated memory sequence.");
            }
            #endif

            // Container is full -> need to insert into descendant
            if (base_type::_children.size() >= type_trait::max_nodes_per_layer) {

                // Preference every other node than shadow node
                for (auto& entry : base_type::_children) {
                    auto resolved = entry.get();

                    if (resolved->isShadowNode())
                        continue;

                    if constexpr (type_trait::distinct_intersect) {
                        if (resolved->intersectsFully(boundary_)) {
                            return resolved->push(element_, boundary_, factory_), ++base_type::_size;
                        }
                    } else {
                        // TODO: consider memory pressure
                        if (resolved->intersectsFully(boundary_) && resolved->push(element_, boundary_, factory_)) {
                            ++base_type::_size;
                            return true;
                        }
                    }
                }

                // Fallback to shadow node
                u64 lowestSize = ~0;
                typename base_type::container_type::iterator cur = base_type::_children.end();

                const typename base_type::container_type::iterator end = base_type::_children.end();
                typename base_type::container_type::iterator iter = base_type::_children.begin();

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
                return cur->get()->push(element_, boundary_, factory_), ++base_type::_size;
            }

            // Container has some space left -> insert directly
            auto result = factory_->assembleShadow();
            result.body->transform() = _STD move(data_->transform);
            result.body->bounding() = _STD move(data_->bounding);
            result.body->payload() = _STD move(data_->payload);

            return base_type::_children.push(_STD move(result.head)), ++base_type::_size;
            #endif
            throw NotImplementedException {};
        }

        template <class FactoryType_>
        bool pop(SceneNodePath path_, const ptr<PayloadType_> element_, const ptr<const FactoryType_> factory_) {

            if (base_type::isLeaf()) {
                return popLeaf(element_);
            }

            const auto decision { path_.pop() };
            auto* const next { base_type::_children[decision].get(factory_->getNodeStorage()) };

            return next->pop(path_, element_, factory_);
        }

        /**
         * Pulls the given nodeId_
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  nodeId_ Identifier for the node.
         *
         * @returns A pull_result_type.
         */
        [[nodiscard]] pull_result_type pull(cref<SceneNodeId> nodeId_) noexcept {

            #if FALSE
            if (base_type::isLeaf()) {
                return {};
            }

            auto iter = base_type::_children.begin();
            const auto last = base_type::_children.end();

            // Check for direct child node
            for (; iter != last; ++iter) {
                if (iter->nodeId() == nodeId_)
                    break;
            }

            // Check for indirect childs in sub-graph
            if (iter == last) {
                pull_result_type result {};
                for (iter = base_type::_children.begin(); iter != last; ++iter) {
                    if (result = iter->get()->pull(nodeId_)) {
                        --base_type::_size;
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

            if (!result.value->get<base_type>()->pullChained(*this, iter.pointer())) {
            #ifdef _DEBUG
                // Attention: This will invalidate the target iterator cause we modify the underlying container memory sequence
                ASSERT(base_type::_children.remove(iter))
            #else
        _children.remove(iter);
            #endif
            }

            if (result.result) {
                --base_type::_size;
            }
            return result;
            #endif

            throw NotImplementedException {};
        }

        /**
         * Pulls this chained
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param parent_ The parent.
         * @param pos_ The position.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool pullChained(cref<base_type> parent_, ptr<SceneNodeHead> pos_) noexcept {

            #if FALSE
            if (base_type::isLeaf()) {
                return false;
            }

            // In-Place replace with [biggest or leaf] shadow node might be best
            typename base_type::container_type::iterator target = base_type::_children.end();
            u64 maxSize = 0;

            auto iter = base_type::_children.begin();
            const auto last = base_type::_children.end();

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
            using value_type = typename base_type::container_type::value_type;

            pos_->value_type::~value_type();
            new(pos_) value_type { _STD move(*target) };

            // Check whether container element was replace or needs to be removed
            ref<base_type> targetNode { *pos_->get() };

            if (!targetNode.pullChained(*this, target.pointer())) {
            #ifdef _DEBUG
                // Attention: This will invalidate the target iterator cause we modify the underlying container memory sequence
                ASSERT(base_type::_children.remove(target))
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
            targetNode.children() = _STD move(base_type::_children);
            --base_type::_size;
            return true;

            #endif

            throw NotImplementedException {};
        }

        [[nodiscard]] bool intersects(cref<base_type> node_) const noexcept {
            // Any node_ will intersect with shadow node
            return true;
        }

        [[nodiscard]] bool intersects(cref<math::Bounding> boundary_) const noexcept {
            // Any bounding and transform will intersect with shadow node
            return true;
        }

        [[nodiscard]] bool intersectsFully(cref<base_type> node_) const noexcept {
            // Any node_ will intersect with shadow node
            return true;
        }

        [[nodiscard]] bool intersectsFully(cref<math::Bounding> boundary_) const noexcept {
            // Any bounding and transform will intersect with shadow node
            return true;
        }
    };

    template <class PayloadType_>
    ptr<const typename SceneNode<PayloadType_>::shadow_type> SceneNode<PayloadType_>::asShadow() const noexcept {
        return ShadowSceneNode<PayloadType_>::castFrom(this);
    }

    template <class PayloadType_>
    ptr<typename SceneNode<PayloadType_>::shadow_type> SceneNode<PayloadType_>::asShadow() noexcept {
        return ShadowSceneNode<PayloadType_>::castFrom(this);
    }

}
