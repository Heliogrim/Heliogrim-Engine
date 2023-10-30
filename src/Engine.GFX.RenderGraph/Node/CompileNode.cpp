#include "CompileNode.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

CompileNode::CompileNode(mref<smr<Auxiliary>> auxiliary_) noexcept :
    Node(_STD move(auxiliary_)) {}
