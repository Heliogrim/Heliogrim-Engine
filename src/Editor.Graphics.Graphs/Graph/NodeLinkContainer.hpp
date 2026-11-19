#pragma once

#include <Engine.Common/__macro.hpp>

#include "../Node/NodeLink.hpp"
#include "../Node/__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::graphics::graph {
    class NodeLinkContainer {
    public:
        using this_type = NodeLinkContainer;

    public:
        NodeLinkContainer() = default;

        NodeLinkContainer(mref<this_type>) noexcept = delete;

        NodeLinkContainer(cref<this_type>) = delete;

        ~NodeLinkContainer() {
            for (const auto* link : links) {
                delete link;
            }
        }

    public:
        ref<this_type> operator=(mref<this_type>) noexcept = delete;

        ref<this_type> operator=(cref<this_type>) = delete;

    public:
        CompactArray<ptr<NodeLink>> links;

        template <class NodeLinkType_ = NodeLink, typename... Args_> requires
            std::derived_from<NodeLinkType_, NodeLink> &&
            std::is_constructible_v<NodeLinkType_, Args_...>
        [[nodiscard]] non_owning_rptr<NodeLinkType_> makeNodeLink(Args_&&... args_) {
            auto* tmp = make_ptr<NodeLinkType_, Args_...>(std::forward<Args_>(args_)...);
            links.push_back(tmp);
            return tmp;
        }
    };
}
