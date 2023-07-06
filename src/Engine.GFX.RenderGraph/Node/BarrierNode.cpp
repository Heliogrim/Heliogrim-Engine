#include "BarrierNode.hpp"

#include <Engine.Common/Make.hpp>

#include "../Component/BarrierComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

BarrierNode::BarrierNode() noexcept :
    Node(
        make_smr<Auxiliary>()
    ) {
    _auxiliary->add(make_uptr<BarrierComponent>());
}

void BarrierNode::traverse(ref<Visitor> visitor_) {}
