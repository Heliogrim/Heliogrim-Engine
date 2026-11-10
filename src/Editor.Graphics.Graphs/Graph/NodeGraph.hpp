#pragma once

#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "../Node/__fwd.hpp"

#include "NodeContainer.hpp"
#include "NodeLinkContainer.hpp"

namespace hg::editor::gfx::graph {
    class NodeGraph {
    private:
        NodeContainer _nodes;
        NodeLinkContainer _links;

    public:
        [[nodiscard]] cref<NodeContainer> nodes() const noexcept;

        [[nodiscard]] ref<NodeContainer> nodes() noexcept;

        [[nodiscard]] cref<NodeLinkContainer> links() const noexcept;

        [[nodiscard]] ref<NodeLinkContainer> links() noexcept;

    public:
        CompactArray<ptr<const Node>> _inputs;
        CompactArray<ptr<const Node>> _outputs;

    public:
        void exposeNodes() noexcept;
    };
}
