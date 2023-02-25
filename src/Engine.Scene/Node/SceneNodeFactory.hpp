#pragma once

#ifdef _DEBUG
#include <cassert>
#endif

#include "SceneNodeHead.hpp"
#include "SceneNode.hpp"

namespace ember::engine::scene {
    template <class NodeStorageType_, class ElementStorageType_, class Traits_>
    class SceneNodeFactory {
    public:
        using this_type = SceneNodeFactory<NodeStorageType_, ElementStorageType_, Traits_>;

        using node_storage_type = NodeStorageType_;
        using element_storage_type = ElementStorageType_;
        using traits = Traits_;

        using node_type = typename node_storage_type::value_type;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 16.08.2021
         */
        SceneNodeFactory() = delete;

        SceneNodeFactory(
            const ptr<node_storage_type> nodeStorage_,
            const ptr<element_storage_type> elementStorage_
        ) noexcept :
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

        [[nodiscard]] const ptr<node_storage_type> getNodeStorage() const noexcept {
            return _nodeStorage;
        }

    public:
        using factory_assemble_result = struct {
            SceneNodeHead heads[traits::max_childs_per_node];
            ptr<node_type> bodys[traits::max_childs_per_node];
        };

    public:
        [[nodiscard]] _STD pair<SceneNodeHead, ptr<node_type>> assembleRoot() const {
            auto nodeId = _nodeIdGen.fetch_add(1);
            auto stored = _nodeStorage->insert(nodeId, {});

            if (!stored.second) {
                throw _STD bad_alloc();
            }

            return _STD make_pair(SceneNodeHead { nodeId }, stored.first);
        }

        [[nodiscard]] factory_assemble_result assembleShadows() const {

            constexpr auto nodes_to_assemble { traits::max_childs_per_node };
            factory_assemble_result result {};

            const auto firstId { _nodeIdGen.fetch_add(nodes_to_assemble) };
            for (auto i = 0; i < nodes_to_assemble; ++i) {

                auto stored { _nodeStorage->insert(firstId + i, {}) };
                DEBUG_ASSERT(stored.second, "Failed to allocate scenen node.")

                result.heads[i] = { firstId + i };
                result.bodys[i] = stored.first;
            }

            return result;
        }

        [[nodiscard]] factory_assemble_result assembleLoosy() const {
            throw NotImplementedException();
        }

        [[nodiscard]] factory_assemble_result assembleSpartials() const {
            throw NotImplementedException();
        }

        [[nodiscard]] factory_assemble_result assembleNaturals(cref<math::Bounding> boundary_) const {
            throw NotImplementedException();
        }

    private:
        // Temporary
        inline static _STD atomic_uint32_t _nodeIdGen { _STD atomic_uint32_t(1) };
    };
}
