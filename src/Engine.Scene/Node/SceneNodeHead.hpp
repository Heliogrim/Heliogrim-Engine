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
    template <class PayloadType_>
    class SceneNode;

    class SceneNodeHead {
    public:
        template <class PayloadType_>
        friend class SceneNode;

    public:
        using this_type = SceneNodeHead;
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
        SceneNodeHead(cref<SceneNodeHead> other_) noexcept;

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
         * Copy Assignment
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        ref<SceneNodeHead> operator=(cref<SceneNodeHead> other_) noexcept;

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
        ref<SceneNodeHead> operator=(mref<SceneNodeHead> other_) noexcept;

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
        mutable ptr<void> _cachedPtr;

        /**
         * The cache version
         */
        mutable smart_version_type _cacheVersion;

    public:
        template <class NodeStorageType_>
        [[nodiscard]] ptr<typename NodeStorageType_::value_type> get(
            const ptr<NodeStorageType_> storage_) const noexcept {

            if (_cachedPtr == nullptr) {
                _cachedPtr = storage_->get(_nodeId);

                if (_cachedPtr == nullptr) {
                    return nullptr;
                }

                _cacheVersion = static_cast<ptr<typename NodeStorageType_::value_type>>(_cachedPtr)->_version;
            }

            return static_cast<ptr<typename NodeStorageType_::value_type>>(_cachedPtr);
        }
    };
}
