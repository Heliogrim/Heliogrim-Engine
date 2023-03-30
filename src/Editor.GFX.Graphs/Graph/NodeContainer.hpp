#pragma once

#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Make.hpp>

#include "../Node/__fwd.hpp"
#include "../Node/Node.hpp"

namespace hg::editor::gfx::graph {
    class NodeContainer {
    public:
        using this_type = NodeContainer;

    public:
        NodeContainer() = default;

        NodeContainer(mref<this_type>) noexcept = delete;

        NodeContainer(cref<this_type>) = delete;

        ~NodeContainer() {
            for (const auto* node : nodes) {
                delete node;
            }
        }

    public:
        ref<this_type> operator=(mref<this_type>) noexcept = delete;

        ref<this_type> operator=(cref<this_type>) = delete;

    public:
        CompactArray<ptr<Node>> nodes;

        template <class NodeType_, typename... Args_> requires
            _STD derived_from<NodeType_, Node> &&
            _STD is_constructible_v<NodeType_, Args_...>
        [[nodiscard]] non_owning_rptr<NodeType_> makeNode(Args_&&... args_) {

            ptr<NodeType_> tmp = make_ptr<NodeType_, Args_...>(_STD forward<Args_>(args_)...);
            tmp->regenerate();

            /**/

            nodes.push_back(tmp);
            return tmp;
        }
    };
}
