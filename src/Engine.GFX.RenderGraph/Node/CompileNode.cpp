#include "CompileNode.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

CompileNode::CompileNode(mref<smr<Auxiliary>> auxiliary_) noexcept :
    Node(std::move(auxiliary_)) {}
