#include "PartialGeneric.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

smr<GenericMeshIG> PartialGeneric::operator()(mref<smr<graph::SubpassNode>> node_) const noexcept {
    return make_smr<GenericMeshIG>();
}
