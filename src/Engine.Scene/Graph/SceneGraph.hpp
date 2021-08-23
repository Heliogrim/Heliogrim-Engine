#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

#include "../Node/SceneNodeConsumer.hpp"
#include "../Node/SceneNodeHead.hpp"
#include "../Storage/EmberSceneNodeStorage.hpp"

namespace ember::engine::scene {

    class RefSceneGraph {
    public:
        using value_type = RefSceneGraph;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;
        using pointer_type = ptr<value_type>;

        using root_type = ref<SceneNodeHead>;
        using storage_type = ref<EmberSceneNodeStorage>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 15.08.2021
         */
        RefSceneGraph() = delete;

        /**
         * Value Copy Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param [in,out] root_ The root.
         * @param [in,out] storage_ The storage.
         */
        RefSceneGraph(
            ref<_STD remove_reference_t<root_type>> root_,
            ref<_STD remove_reference_t<storage_type>> storage_) noexcept (
            _STD is_nothrow_copy_constructible_v<root_type> &&
            _STD is_nothrow_copy_constructible_v<storage_type>
        );

        /**
         * Value Move Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  root_ The root.
         * @param  storage_ The storage.
         */
        RefSceneGraph(
            mref<_STD remove_reference_t<root_type>> root_,
            mref<_STD remove_reference_t<storage_type>> storage_) noexcept (
            _STD is_nothrow_move_constructible_v<root_type> &&
            _STD is_nothrow_move_constructible_v<storage_type>
        );

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         */
        RefSceneGraph(const_reference_type other_) noexcept = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         */
        RefSceneGraph(mref<value_type> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 15.08.2021
         */
        ~RefSceneGraph() noexcept;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) noexcept = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type>) noexcept = delete;

    private:
        /**
         * Scene Graph Root
         */
        root_type _root;

    public:
        /**
         * Gets the root
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @returns A cref&lt;_STD remove_reference_t&lt;root_type&gt;&gt;
         */
        [[nodiscard]] cref<_STD remove_reference_t<root_type>> root() const noexcept;

        /**
         * Gets the root
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @returns A ref&lt;_STD remove_reference_t&lt;root_type&gt;&gt;
         */
        [[nodiscard]] cref<_STD remove_reference_t<root_type>> root() noexcept;

    private:
        /**
         * Scene Graph Storage
         */
        storage_type _storage;

    public:
        /**
         * Traversals the graph with the given consumer
         *
         * @tparam Consumer Type of the consumer.
         * @param  consumer_ The consumer.
         */
        template <typename Consumer> requires IsSceneNodeConsumer<Consumer>
        void traversal(cref<Consumer> consumer_) const {
            return traversal(static_cast<_STD function<bool(cref<SceneNode>)>>(consumer_));
        }

        /**
         * Traversals the graph with the given consumer
         *
         * @author Julius
         * @date 09.01.2021
         *
         * @param  consumer_ The consumer.
         */
        void traversal(cref<_STD function<bool(cref<SceneNode>)>> consumer_) const;
    };

    class SceneGraph {
    public:
        using value_type = SceneGraph;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;
        using pointer_type = ptr<value_type>;

        using root_type = SceneNodeHead;
        using storage_type = sptr<EmberSceneNodeStorage>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 15.08.2021
         */
        SceneGraph() = delete;

        /**
         * Value Copy Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  root_ The root.
         * @param  storage_ The storage.
         */
        SceneGraph(
            cref<_STD remove_reference_t<root_type>> root_,
            cref<_STD remove_reference_t<storage_type>> storage_) noexcept (
            _STD is_nothrow_copy_constructible_v<root_type> &&
            _STD is_nothrow_copy_constructible_v<storage_type>
        );

        /**
         * Value Move Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  root_ The root.
         * @param  storage_ The storage.
         */
        SceneGraph(
            mref<_STD remove_reference_t<root_type>> root_,
            mref<_STD remove_reference_t<storage_type>> storage_) noexcept (
            _STD is_nothrow_move_constructible_v<root_type> &&
            _STD is_nothrow_move_constructible_v<storage_type>
        );

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         */
        SceneGraph(const_reference_type other_) noexcept = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         */
        SceneGraph(mref<value_type> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 15.08.2021
         */
        ~SceneGraph() noexcept;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type other_) noexcept = default;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type> other_) noexcept = default;

    private:
        /**
         * Scene Graph Root
         */
        root_type _root;

    public:
        /**
         * Gets the root
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @returns A cref&lt;_STD remove_reference_t&lt;root_type&gt;&gt;
         */
        [[nodiscard]] cref<_STD remove_reference_t<root_type>> root() const noexcept;

        /**
         * Gets the root
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @returns A ref&lt;_STD remove_reference_t&lt;root_type&gt;&gt;
         */
        [[nodiscard]] ref<_STD remove_reference_t<root_type>> root() noexcept;

    private:
        /**
         * Scene Graph Storage
         */
        storage_type _storage;

    public:
        /**
         * Gets the storage
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A cref&lt;storage_type&gt;
         */
        [[nodiscard]] cref<storage_type> storage() const noexcept;

    public:
        /**
         * Converts this to a reference scene graph
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @returns A RefSceneGraph.
         */
        [[nodiscard]] RefSceneGraph asRefSceneGraph() const;

        /**
         * Converts this to a reference scene graph
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A RefSceneGraph.
         */
        [[nodiscard]] RefSceneGraph asRefSceneGraph();

    public:
        /**
         * Traversals the graph with the given consumer
         *
         * @tparam Consumer Type of the consumer.
         * @param  consumer_ The consumer.
         */
        template <typename Consumer> requires IsSceneNodeConsumer<Consumer>
        void traversal(cref<Consumer> consumer_) const {
            return traversal(static_cast<_STD function<bool(cref<SceneNode>)>>(consumer_));
        }

        /**
         * Traversals the graph with the given consumer
         *
         * @author Julius
         * @date 09.01.2021
         *
         * @param  consumer_ The consumer.
         */
        void traversal(cref<_STD function<bool(cref<SceneNode>)>> consumer_) const;
    };

}
