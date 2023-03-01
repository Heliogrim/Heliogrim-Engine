#pragma once

#include "SceneNode.hpp"

namespace hg::engine::scene {
    template <class PayloadType_, typename NodeSubType_>
    class SceneNodeSubBase :
        public SceneNode<PayloadType_> {
    public:
        using this_type = SceneNodeSubBase<PayloadType_, NodeSubType_>;
        using base_type = SceneNode<PayloadType_>;

    public:
        SceneNodeSubBase() noexcept = default;

    public:
        template <class OtherType> requires std::is_base_of_v<SceneNode<PayloadType_>, OtherType>
        static ptr<const NodeSubType_> castFrom(ptr<const OtherType> other_) {
            if (other_->nodeState() == NodeSubType_::type_trait::value) {
                return static_cast<ptr<const NodeSubType_>>(
                    static_cast<ptr<const void>>(other_)
                );
            }
            return nullptr;
        }

        template <class OtherType> requires std::is_base_of_v<SceneNode<PayloadType_>, OtherType>
        static ptr<NodeSubType_> castFrom(ptr<OtherType> other_) {
            if (other_->nodeState() == NodeSubType_::type_trait::value) {
                return static_cast<ptr<NodeSubType_>>(
                    static_cast<ptr<void>>(other_)
                );
            }
            return nullptr;
        }
    };
}
