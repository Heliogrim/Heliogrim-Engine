#pragma once

#include "SceneNodeHead.hpp"
#include "SceneNode.hpp"

namespace ember::engine::scene {

    template <class NodeStorageType_, class ElementStorageType_>
    class SceneNodeFactory {
    public:
        using this_type = SceneNodeFactory<NodeStorageType_, ElementStorageType_>;

        using node_storage_type = NodeStorageType_;
        using element_storage_type = ElementStorageType_;

        using node_type = typename node_storage_type::value_type;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 16.08.2021
         */
        SceneNodeFactory() = delete;

        SceneNodeFactory(const ptr<node_storage_type> nodeStorage_,
            const ptr<element_storage_type> elementStorage_) noexcept :
            _nodeStorage(nodeStorage_),
            _elementStorage(elementStorage_) {}

        /**
         * Destructor
         *
         * @author Julius
         * @date 16.08.2021
         */
        ~SceneNodeFactory() noexcept = default;

    private:
        ptr<node_storage_type> _nodeStorage;
        ptr<element_storage_type> _elementStorage;

    public:
        [[nodiscard]] const ptr<element_storage_type> getElementStorage() const noexcept {
            return _elementStorage;
        }

    public:
        using factory_assemble_result = struct {
            SceneNodeHead head;
            ptr<node_type> body;
        };

    public:
        [[nodiscard]] factory_assemble_result assembleRoot() const {
            return assembleShadow();
        }

        [[nodiscard]] factory_assemble_result assembleShadow() const {

            auto nodeId = _nodeIdGen.fetch_add(1);
            auto stored = _nodeStorage->insert(nodeId, {});

            if (!stored.second) {
                throw _STD bad_alloc();
            }

            return {
                { nodeId },
                stored.first
            };
        }

        [[nodiscard]] factory_assemble_result assembleLoosy() const {
            throw NotImplementedException();
        }

        [[nodiscard]] factory_assemble_result assembleSpartial() const {
            throw NotImplementedException();
        }

        [[nodiscard]] factory_assemble_result assembleNatural() const {
            throw NotImplementedException();
        }

    private:
        // Temporary
        inline static _STD atomic_uint32_t _nodeIdGen { _STD atomic_uint32_t(1) };
    };
}
