#include "RuntimeNode.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

RuntimeNode::RuntimeNode() noexcept :
    Node() {}

RuntimeNode::RuntimeNode(mref<smr<Auxiliary>> auxiliary_) noexcept :
    Node(_STD move(auxiliary_)) {}
