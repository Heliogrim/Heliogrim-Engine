#include "PartialProcedural.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

smr<ProcMeshIG> PartialProcedural::operator()(mref<smr<graph::SubpassNode>> node_) const noexcept {
    return make_smr<ProcMeshIG>();
}
