#include "PartialScene.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

smr<SceneMeshIG> PartialScene::operator()(mref<smr<graph::SubpassNode>> node_) const noexcept {
    return make_smr<SceneMeshIG>();
}
