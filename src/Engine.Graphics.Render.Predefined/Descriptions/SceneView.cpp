#include "SceneView.hpp"

#include <Engine.Graphics.RenderGraph/Relation/SceneViewDescription.hpp>

import Engine.Common;

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Description> engine::render::makeSceneViewDescription() {
    static auto obj = make_smr<graph::SceneViewDescription>();
    return clone(obj).into<graph::Description>();
}
