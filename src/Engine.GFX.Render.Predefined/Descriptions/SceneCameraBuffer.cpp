#include "SceneCameraBuffer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/Relation/SceneCameraDescription.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Description> engine::render::makeSceneCameraBufferDescription() {
    static auto obj = make_smr<graph::SceneCameraDescription>();
    return clone(obj).into<graph::Description>();
}
