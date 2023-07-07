#include "Node.hpp"

#include "../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

Node::Node(mref<smr<Auxiliary>> auxiliary_) noexcept :
    _auxiliary(_STD move(auxiliary_)) {}

smr<Auxiliary> Node::auxiliary() const noexcept {
    return _auxiliary;
}
