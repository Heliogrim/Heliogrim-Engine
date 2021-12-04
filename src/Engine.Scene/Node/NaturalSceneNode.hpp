#pragma once

#include "SceneNodeSubBase.hpp"

namespace ember::engine::scene {

    template <class PayloadType_>
    class NaturalSceneNode final :
        public SceneNodeSubBase<PayloadType_, NaturalSceneNode<PayloadType_>> {
    public:
        using type_trait = scene_node_traits<SceneNodeState::eNatural>;

        using this_type = NaturalSceneNode<PayloadType_>;
        using underlying_type = SceneNodeSubBase<PayloadType_, this_type>;
        using base_type = SceneNode<PayloadType_>;

    public:
        /**
         * Default Constructor
         *
         * @author Julius
         * @date 05.11.2021
         */
        NaturalSceneNode() noexcept :
            underlying_type() {
            base_type::_state = SceneNodeState::eNatural;
        }

    public:
        /**
         * Query if this is natural node
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns True if natural node, false if not.
         */
        [[nodiscard]] constexpr bool isNaturalNode() const noexcept {
            return true;
        }

    public:
        [[nodiscard]] bool contains(cref<SceneNodeId> nodeId_) const noexcept {
            throw NotImplementedException();
        }

        [[nodiscard]] bool contains(cref<base_type> other_) const noexcept {
            #if FALSE
            if (isLeaf()) {
                return false;
            }

            // TODO: Replace
            for (const auto& entry : _children) {
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

            throw NotImplementedException();
        }

        template <class FactoryType_>
        bool push(const ptr<PayloadType_> element_, cref<math::Bounding> boundary_,
            const ptr<const FactoryType_> factory_) {

            #if FALSE
            #ifdef _DEBUG
            if (_children.empty() && _children.unsafe_base() == nullptr) {
                throw _STD runtime_error("Node Container does not contain a valid or pre-allocated memory sequence.");
            }
            #endif

            if (isLeaf()) {

                // If this is empty, push payload to this node
                if (_payload.empty()) {
                    _payload = _STD move(data_.payload);

                    ++_size;
                    return true;

                } else {

                    const auto halfs = _bounding.extent() / 2.0F;
                    const auto& center = _bounding.center();

                    // If this is not empty, split this node into childs
                    for (u8 i = 0; i < 8ui8; ++i) {

                        const math::vec3 dir {
                            i & 0b0001 ? -1.F : 1.F,
                            i & 0b0010 ? -1.F : 1.F,
                            i & 0b0100 ? -1.F : 1.F
                        };

                        auto result = factory_->assembleNatural();

                        // TODO: result.body->transform() = _transform;
                        result.body->bounding() = {
                            center + (halfs * dir),
                            halfs
                        };

                        _children.push(_STD move(result.head));
                    }

                    // Recursive push due to changed leaf state
                    return push(_STD move(data_), factory_);
                }

            } else {

                for (auto& entry : _children) {
                    auto resolved = entry.get();

                    if (resolved->intersectsFully(data_)) {
                        return resolved->push(_STD move(data_), factory_), ++_size;
                    }
                }

                throw _STD runtime_error("Unable to sustain natural scene tree.");

            }
            #endif

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
    ptr<const typename SceneNode<PayloadType_>::natural_type> SceneNode<PayloadType_>::asNatural() const noexcept {
        return NaturalSceneNode<PayloadType_>::castFrom(this);
    }

    template <class PayloadType_>
    ptr<typename SceneNode<PayloadType_>::natural_type> SceneNode<PayloadType_>::asNatural() noexcept {
        return NaturalSceneNode<PayloadType_>::castFrom(this);
    }

}
