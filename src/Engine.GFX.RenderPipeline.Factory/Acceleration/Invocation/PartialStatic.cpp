#include "PartialStatic.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

smr<StaticMeshIG> PartialStatic::operator()(mref<smr<graph::SubpassNode>> node_) const noexcept {
    return make_smr<StaticMeshIG>();
}
