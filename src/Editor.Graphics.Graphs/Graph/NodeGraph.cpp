#include "NodeGraph.hpp"

using namespace hg::editor::gfx::graph;
using namespace hg;

[[nodiscard]] static bool isInputNode(const ptr<const Node> node_);

[[nodiscard]] static bool isOutputNode(const ptr<const Node> node_);

cref<NodeContainer> NodeGraph::nodes() const noexcept {
    return _nodes;
}

ref<NodeContainer> NodeGraph::nodes() noexcept {
    return _nodes;
}

cref<NodeLinkContainer> NodeGraph::links() const noexcept {
    return _links;
}

ref<NodeLinkContainer> NodeGraph::links() noexcept {
    return _links;
}

void NodeGraph::exposeNodes() noexcept {

    _inputs.clear();
    _outputs.clear();

    /**/

    for (const auto& node : _nodes.nodes) {

        if (isInputNode(node)) {
            _inputs.push_back(node);
        }

        if (isOutputNode(node)) {
            _outputs.push_back(node);
        }

    }
}

[[nodiscard]] static bool isInputNode(const ptr<const Node> node_) {

    // TODO: Replace with reflected type id checks
    const auto typeName = node_->getNodeTypeName();

    if (typeName == "SubGraphInputNode"sv) {
        return true;
    }
    return false;
}

[[nodiscard]] static bool isOutputNode(const ptr<const Node> node_) {

    // TODO: Replace with reflected type id checks
    const auto typeName = node_->getNodeTypeName();

    if (typeName == "SubGraphOutputNode"sv) {
        return true;
    }
    return false;
}
