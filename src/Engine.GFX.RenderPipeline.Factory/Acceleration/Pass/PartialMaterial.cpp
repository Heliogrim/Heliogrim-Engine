#include "PartialMaterial.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

smr<MaterialSubpassStage> PartialMaterial::operator()(
    mref<smr<graph::SubpassNode>> node_,
    mref<uptr<InvocationGenerator>> ig_
) const noexcept {
    return make_smr<MaterialSubpassStage>();
}
