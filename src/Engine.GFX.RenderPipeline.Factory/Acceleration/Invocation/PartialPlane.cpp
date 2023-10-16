#include "PartialPlane.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

smr<PlaneMeshIG> PartialPlane::operator()(mref<smr<graph::SubpassNode>> node_) const noexcept {
    return make_smr<PlaneMeshIG>();
}
