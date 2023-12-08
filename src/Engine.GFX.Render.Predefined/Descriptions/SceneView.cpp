#include "SceneView.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/Relation/SceneViewDescription.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Description> engine::render::makeSceneViewDescription() {
    static auto obj = make_smr<graph::SceneViewDescription>();
    return clone(obj).into<graph::Description>();
}
