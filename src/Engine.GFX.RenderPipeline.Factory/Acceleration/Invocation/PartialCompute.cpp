#include "PartialCompute.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

smr<ComputeIG> PartialCompute::operator()(mref<smr<graph::SubpassNode>> node_) const noexcept {
    return make_smr<ComputeIG>();
}
