#pragma once

#include "SceneNodeSubBase.hpp"

namespace hg::engine::scene {
    template <class PayloadType_>
    class SpartialSceneNode final :
        public SceneNodeSubBase<PayloadType_, SpartialSceneNode<PayloadType_>> {
    public:
        using type_trait = scene_node_traits<SceneNodeState::eSpartial>;

        using this_type = SpartialSceneNode<PayloadType_>;
        using underlying_type = SceneNodeSubBase<PayloadType_, this_type>;
        using base_type = SceneNode<PayloadType_>;

    public:
        [[nodiscard]] constexpr bool isSpartialNode() const noexcept {
            return true;
        }

    public:
        [[nodiscard]] bool contains(cref<SceneNodeId> nodeId_) const noexcept {
            throw NotImplementedException();
        }

        [[nodiscard]] bool contains(cref<base_type> other_) const noexcept {
            throw NotImplementedException();
        }

        template <class FactoryType_>
        bool push(
            const ptr<PayloadType_> element_,
            cref<math::Bounding> boundary_,
            const ptr<const FactoryType_> factory_,
            SceneNodePath forwardPath_
        ) {
            throw NotImplementedException();
        }

        template <class FactoryType_>
        bool pop(SceneNodePath path_, const ptr<PayloadType_> element_, const ptr<const FactoryType_> factory_) {
            throw NotImplementedException();
        }

        [[nodiscard]] pull_result_type pull(cref<SceneNodeId> nodeId_) noexcept {
            throw NotImplementedException();
        }

        bool pullChained(cref<base_type> parent_, ptr<SceneNodeHead> pos_) noexcept {
            throw NotImplementedException();
        }

        [[nodiscard]] bool intersects(cref<base_type> node_) const noexcept {
            throw NotImplementedException();
        }

        [[nodiscard]] bool intersects(cref<math::Bounding> boundary_) const noexcept {
            throw NotImplementedException();
        }

        [[nodiscard]] bool intersectsFully(cref<base_type> node_) const noexcept {
            throw NotImplementedException();
        }

        [[nodiscard]] bool intersectsFully(cref<math::Bounding> boundary_) const noexcept {
            throw NotImplementedException();
        }
    };

    template <class PayloadType_>
    ptr<const typename SceneNode<PayloadType_>::spartial_type> SceneNode<PayloadType_>::asSpartial() const noexcept {
        return SpartialSceneNode<PayloadType_>::castFrom(this);
    }

    template <class PayloadType_>
    ptr<typename SceneNode<PayloadType_>::spartial_type> SceneNode<PayloadType_>::asSpartial() noexcept {
        return SpartialSceneNode<PayloadType_>::castFrom(this);
    }
}
