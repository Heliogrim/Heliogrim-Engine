#include "SceneCameraBuffer.hpp"

#include <Engine.Graphics.RenderGraph/Relation/SceneCameraDescription.hpp>

import Engine.Common;

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Description> engine::render::makeSceneCameraBufferDescription() {
    static auto obj = make_smr<graph::SceneCameraDescription>();
    return clone(obj).into<graph::Description>();
}
