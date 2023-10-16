#include "PartialSkeletal.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

smr<SkeletalMeshIG> PartialSkeletal::operator()(mref<smr<graph::SubpassNode>> node_) const noexcept {
    return make_smr<SkeletalMeshIG>();
}
