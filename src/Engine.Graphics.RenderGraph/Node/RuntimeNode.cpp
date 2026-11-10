#include "RuntimeNode.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

RuntimeNode::RuntimeNode() noexcept :
    Node() {}

RuntimeNode::RuntimeNode(mref<smr<Auxiliary>> auxiliary_) noexcept :
    Node(std::move(auxiliary_)) {}
