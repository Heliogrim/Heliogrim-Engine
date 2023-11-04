#include "SceneView.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/Relation/SceneViewDescription.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const graph::Description> engine::gfx::render::makeSceneViewDescription() {
    static auto obj = make_smr<graph::SceneViewDescription>();
    return clone(obj).into<graph::Description>();
}
