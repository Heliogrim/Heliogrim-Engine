#pragma once

#include "SceneNode.hpp"

namespace ember::engine::scene {
    template <typename SubType>
    class SceneNodeSubBase :
        public SceneNode {
    public:
        SceneNodeSubBase() noexcept = default;

    public:
        template <class OtherType> requires std::is_base_of_v<SceneNode, OtherType>
        static ptr<const SubType> castFrom(ptr<const OtherType> other_) {
            if (other_->nodeState() == SubType::type_trait::value) {
                return static_cast<ptr<const SubType>>(
                    static_cast<ptr<const void>>(other_)
                );
            }
            return nullptr;
        }

        template <class OtherType> requires std::is_base_of_v<SceneNode, OtherType>
        static ptr<SubType> castFrom(ptr<OtherType> other_) {
            if (other_->nodeState() == SubType::type_trait::value) {
                return static_cast<ptr<SubType>>(
                    static_cast<ptr<void>>(other_)
                );
            }
            return nullptr;
        }
    };
}
