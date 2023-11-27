#include "SceneCameraBuffer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/Relation/SceneCameraDescription.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const graph::Description> engine::gfx::render::makeSceneCameraBufferDescription() {
    static auto obj = make_smr<graph::SceneCameraDescription>();
    return clone(obj).into<graph::Description>();
}
