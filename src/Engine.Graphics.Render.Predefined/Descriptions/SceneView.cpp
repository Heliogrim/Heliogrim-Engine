#include "SceneView.hpp"

import Engine.Common;
import Engine.Graphics.RenderGraph;

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Description> engine::render::makeSceneViewDescription() {
    static auto obj = make_smr<graph::SceneViewDescription>();
    return clone(obj).into<graph::Description>();
}
