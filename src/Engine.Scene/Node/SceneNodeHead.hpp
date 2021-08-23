#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "SceneNodeVersion.hpp"
#include "SceneNodeId.hpp"

namespace ember::engine::scene {

    /**
     * Forward Declaration
     *
     * @author Julius
     * @date 15.08.2021
     */
    class SceneNode;

    /**
     * Forward Declaration
     *
     * @author Julius
     * @date 16.08.2021
     */
    class EmberSceneNodeStorage;

    class SceneNodeHead {
    public:
        friend class SceneNode;

    public:
        using value_type = SceneNodeHead;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;
        using pointer_type = ptr<value_type>;

        using smart_version_type = SceneNodeVersion;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 15.08.2021
         */
        SceneNodeHead() noexcept;

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  nodeId_ Identifier for the node.
         */
        SceneNodeHead(cref<SceneNodeId> nodeId_) noexcept;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         */
        SceneNodeHead(const_reference_type other_) noexcept;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         */
        SceneNodeHead(mref<SceneNodeHead> other_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 15.08.2021
         */
        ~SceneNodeHead() noexcept;

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
        reference_type operator=(const_reference_type other_) noexcept;

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
        reference_type operator=(mref<value_type> other_) noexcept;

    private:
        /**
         * Identifier for a mapped node
         */
        SceneNodeId _nodeId;

    public:
        /**
         * Node identifier
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @returns A cref&lt;SceneNodeId&gt;
         */
        [[nodiscard]] cref<SceneNodeId> nodeId() const noexcept;

    private:
        /**
         * The cached pointer
         */
        mutable ptr<SceneNode> _cachedPtr;

        /**
         * The cache version
         */
        mutable smart_version_type _cacheVersion;

    public:
        /**
         * Gets a pointer to the scene node, otherwise throw
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  storage_ The storage where to resolve.
         *
         * @returns A ptr&lt;SceneNode&gt;
         */
        [[nodiscard]] ptr<SceneNode> get(cref<EmberSceneNodeStorage> storage_) const;

        /**
         * Gets a pointer to the scene node, otherwise return nullptr
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  storage_ The storage where to resolve.
         *
         * @returns A ptr&lt;SceneNode&gt;
         */
        [[nodiscard]] ptr<SceneNode> get(cref<EmberSceneNodeStorage> storage_, _STD nothrow_t) const noexcept;

    public:
        // TODO: Temporary
        ptr<EmberSceneNodeStorage> storage;

        [[nodiscard]] ptr<SceneNode> get() const noexcept;
    };
}
